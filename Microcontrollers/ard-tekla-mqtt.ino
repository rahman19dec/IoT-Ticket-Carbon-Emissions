#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "con_secrets.h"

// Wi-Fi Credentials
const char ssid[] = SECRET_SSID;
const char wPass[] = SECRET_PASS;

// Device number (can be dynamically set)
int deviceNumber = 2; 

// MQTT Broker
const char mqttServer[] = "192.168.77.152";
const int mqttPort = 1883;
const char mqttUser[] = MQTT_USR;
const char mqttPass[] = MQTT_PASS;

// Wi-Fi connection attempt limit
const int MAX_WIFI_ATTEMPTS = 10;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Temperature Sensor Variables
#define TEMPSENSOR A0

// Dynamic MQTT Topic
char mqttTopic[50];  // Adjust size based on expected topic length

// Function to connect to Wi-Fi
void connectWiFi() {
    Serial.print("Connecting to Wi-Fi...");
    WiFi.disconnect();
    WiFi.begin(ssid, wPass);
    int attemptCount = 0;
    int sleepTime = 500;
    while (WiFi.status() != WL_CONNECTED && attemptCount < MAX_WIFI_ATTEMPTS) { // Max attempts
        delay(1000);
        attemptCount++;
        Serial.print("Attempt: ");
        Serial.println(attemptCount);

        // Print Wi-Fi status code for debugging
        int status = WiFi.status();
        Serial.print("Status Code: ");
        Serial.println(status);

        // Print Wi-Fi signal strength
        Serial.print("Signal Strength: ");
        Serial.println(WiFi.RSSI());
        delay(sleepTime);
        sleepTime = sleepTime*1.5;
        // Retry if connection fails
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Wi-Fi connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("Failed to connect to Wi-Fi.");
        Serial.println("Restarting....");
        NVIC_SystemReset(); // restart
    }
}

// Function to connect to MQTT broker
void connectMQTT() {
    // Create dynamic client ID string with device number
    char clientID[32]; // Ensure the string buffer is large enough
    snprintf(clientID, sizeof(clientID), "Arduino_d%d", deviceNumber);

    client.setKeepAlive(90);  // Set keep-alive timeout

    while (!client.connected()) {
        if (client.connect(clientID, mqttUser, mqttPass)) {
            Serial.println("Connected to MQTT broker!");
        } else {
            Serial.print("Failed. Error Code: ");
            Serial.print(client.state());
            Serial.println(" Retrying...");
            delay(2000); // Retry after 2 seconds
        }
    }
}

// Function to get temperature from the sensor
float getTemp() {
    float v_in, a, b;
    float temperature = 0.0; // Store the final reading
    float ten_sample_sum = 0.0; // Store sum of 10 samples

    // Take 10 samples from LMT87 temperature sensor
    for (int sample = 0; sample < 10; sample++) {
        // Read voltage and convert to mV
        v_in = 1000.0 * ((float)analogRead(TEMPSENSOR) * 3.3) / 1024.0;
        a = (sqrt(pow(-13.582, 2.0) + (4.0 * 0.00433) * (2230.8 - v_in)));
        b = (2.0 * (-0.00433));
        temperature = (13.582 - a) / b + 30.0;

        // Sample every 0.1 seconds
        delay(100);
        ten_sample_sum += temperature;
    }

    // Return the average temperature
    return (ten_sample_sum / 10.0);
}

void setup() {
    Serial.begin(9600);

    // Ensure the Wi-Fi module is available
    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("Communication with Wi-Fi module failed!");
        while (true); // Halt the program if the Wi-Fi module is unavailable
    }

    // Connect to Wi-Fi
    connectWiFi();

    // Set the dynamic MQTT topic based on the device number
    snprintf(mqttTopic, sizeof(mqttTopic), "arduino/device%d/temp", deviceNumber);

    // Set MQTT server and connect
    client.setServer(mqttServer, mqttPort);
    connectMQTT();
}

unsigned long lastPublishTime = 0;
const unsigned long publishInterval = 30000;  // 30 seconds

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi lost. Reconnecting...");
        connectWiFi();
    }

    if (WiFi.status() == WL_CONNECTED && !client.connected()) {
        Serial.println("MQTT disconnected. Reconnecting...");
        connectMQTT();
    }

    client.loop();  // ✅ Call frequently to keep MQTT connection alive

    // Publish temperature only if 30 seconds have passed
    if (millis() - lastPublishTime >= publishInterval) {
        lastPublishTime = millis();

        float temperatureC = getTemp();
        char tempStr[32];  // Array to hold the formatted string
        snprintf(tempStr, sizeof(tempStr), "Temperature (d%d): %.1f °C", deviceNumber, temperatureC);
        Serial.println(tempStr);

        char tempString[8];
        snprintf(tempString, sizeof(tempString), "%.1f", temperatureC);
        client.publish(mqttTopic, tempString);  // Using dynamic MQTT topic
    }
}
