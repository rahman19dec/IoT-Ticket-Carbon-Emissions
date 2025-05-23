import creds
import requests
import time
from datetime import datetime, timezone
from influxdb import InfluxDBClient  # InfluxDB 1.x client
from requests.auth import HTTPBasicAuth


# Fingrid configuration
dataset_c = 265 # Dataset ID and API URL for carbon consumption
api_key = creds.fingrid_key # API Key

# InfluxDB configuration
influxdb_host = creds.db_host  # hostname or IP address of your InfluxDB server
influxdb_port = 8086        # Default port for InfluxDB
db_name = "asset_data"      # Your InfluxDB database name
db_usr = creds.db_usr       # Your InfluxDB username
db_pass = creds.db_pass     # Your InfluxDB password

# IoT ticket configuration
it_org = creds.it_org
it_user = creds.it_user
it_pass = creds.it_pass

# Function to fetch data from API with retry for rate limits
def fetch_data(dataset_id, retries=5):
    headers = {"x-api-key": api_key}
    params = {"pageSize" : 100}  # Note that new data is updated every 3 minutes
    api_url = f"https://data.fingrid.fi/api/datasets/{dataset_id}/data"
    for attempt in range(retries):
        try:
            response = requests.get(api_url, headers=headers, params=params)
            if response.status_code == 200:
                return response.json()["data"]
            else:
                print(f"Error: {response.status_code}")
                time.sleep(1)  # Wait before retry
        except Exception as e:
            print(f"Error fetching data: {e}")
            time.sleep(1)  # Wait before retry
    return []  # Return empty list if all retries fail

# Fetch data for carbon consumption dataset
data_c = fetch_data(dataset_c)



# Initialize InfluxDB client (InfluxDB 1.x)
client = InfluxDBClient(
        host=influxdb_host,
        port=influxdb_port,
        username=db_usr,
        password=db_pass,
        database=db_name
        )

# Function to push data to IoT ticket 
def push_to_iot_ticket(data):
    org = creds.it_org
    device = "fingrid"
    url = f"https://cloud.iot-ticket.com/http-adapter/telemetry/{org}/{device}"
    tel = []
    for row in data:
        start_time = row['startTime']
        value = float(row['value'])  # Ensure value is always a float
        formatted_time = datetime.strptime(start_time, "%Y-%m-%dT%H:%M:%S.%fZ").replace(tzinfo=timezone.utc).isoformat()
        tel.append({formatted_time:value})
    t = {"t":[
            {"n" : "cfc",
            "dt":"double",
            "unit":"gCO2/kWh",
            "data":tel}
    ]}
    response = requests.put(url, json=t, auth=HTTPBasicAuth(creds.it_user, creds.it_pass))
    if response.status_code == 202 or response.status_code == 200:
        print("Data pushed to IoT ticket successfully.")
    else:
        print(f"Failed to push data to IoT ticket: {response.status_code} - {response.text}")

# Function to push data to InfluxDB
def push_data_to_influxdb(data):
    for row in data:
        start_time = row['startTime']
        value = float(row['value'])  # Ensure value is always a float

        # Convert start_time to correct format
        formatted_time = datetime.strptime(start_time, "%Y-%m-%dT%H:%M:%S.%fZ").replace(tzinfo=timezone.utc).isoformat()

        # Check if data already exists before inserting
        json_body = [
            {
                "measurement": "external",
                "tags": {
                    "source": "Fingrid",
                    "id": f"external_fgrid_cfc_1",
                    "name": f"{'Consumption'} Carbon Factor",
                    "unit": "gCO2/kWh"
                },
                "time": formatted_time,
                "fields": {
                    "value": float(value),
                    "measurement_str": f"Carbon Factor Consumption",
                }
            }
        ]
        client.write_points(json_body)


# Push the carbon consumption data to InfluxDB and IoT ticket
push_data_to_influxdb(data_c)
push_to_iot_ticket(data_c,)


# Close the InfluxDB client
client.close()