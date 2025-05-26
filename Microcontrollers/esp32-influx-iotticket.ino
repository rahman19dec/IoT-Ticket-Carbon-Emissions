#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "secrets.h"
#include "sensors.h"
#include "time.h"

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setupSensors();
}

void loop() {
  float temp, humidity;
  uint16_t co2;
  if (readSensors(temp, humidity, co2)) {
    String timestamp = getTimestampUTC();
    sendToInfluxDB(temp, humidity, co2);
    sendToIotTicket(timestamp, temp, humidity);
  } else {
    Serial.println("Sensor read failed");
  }
  delay(60000); // 1 minute
}

String getTimestampUTC() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "1970-01-01T00:00:00.000Z";
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S.000Z", &timeinfo);
  return String(buffer);
}

void sendToInfluxDB(float temp, float hum, uint16_t co2) {
  HTTPClient http;
  http.begin(INFLUXDB_URL);
  http.setAuthorization(INFLUXDB_USER, INFLUXDB_PASS);
  http.addHeader("Content-Type", "text/plain");

  String payload = "environment temperature=" + String(temp) + 
                   ",humidity=" + String(hum) + 
                   ",co2=" + String(co2);
  int httpCode = http.POST(payload);
  Serial.println("InfluxDB response: " + String(httpCode));
  http.end();
}

void sendToIotTicket(String timestamp, float temp, float hum) {
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure(); // For self-signed certs, not for production

  http.begin(client, IOT_TICKET_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(IOT_TICKET_TOKEN));

  String json = "{ \"t\": [";
  json += "{\"n\":\"Temperature\",\"dt\":\"double\",\"unit\":\"C\",\"data\":[{\"" + timestamp + "\":" + String(temp, 2) + "}]}";
  json += ",";
  json += "{\"n\":\"Humidity\",\"dt\":\"double\",\"unit\":\"\",\"data\":[{\"" + timestamp + "\":" + String(hum, 2) + "}]}";
  json += "]}";

  int httpCode = http.POST(json);
  Serial.println("IoT-Ticket response: " + String(httpCode));
  http.end();
}
