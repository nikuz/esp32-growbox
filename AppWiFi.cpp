#include <Arduino.h>
#include <WiFi.h>

#include "AppWiFi.h"
#include "def.h"
#include "AppWiFiDef.h"

AppWiFi::AppWiFi() {}

AppWiFi::~AppWiFi() {}

void AppWiFi::initiate() {
#if PRODUCTION
    IPAddress ip(192, 168, 1, 100);
#else
    IPAddress ip(192, 168, 1, 101);
#endif
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress dns1(8, 8, 8, 8);  // google DNS
    IPAddress dns2(8, 8, 4, 4);  // google DNS

    // Configure static IP and Google DNS
    WiFi.config(ip, gateway, subnet, dns1, dns2);
    // Connect to provided SSID and PSWD
    // WiFi.begin(SSID, PSWD);
}

bool AppWiFi::isConnected() {
    return WiFi.isConnected();
}

const char *AppWiFi::getSSID() {
    return SSID;
}

const char *AppWiFi::getPSWD() {
    return PSWD;
}

void AppWiFi::reConnect() {
    unsigned long wifiReconnectInterval = 1000L * 5L;
    unsigned long wifiReconnectTime = millis() + wifiReconnectInterval;
    Serial.println("");
    Serial.println("Connecting to " + String(SSID));
    const int maxReconnectAttempts = 5;
    int reconnectAttemptIndex = 0;
    while (!AppWiFi::isConnected() && reconnectAttemptIndex < maxReconnectAttempts) {
        Serial.print(".");  // Keep the serial monitor lit!
        delay(500);
        if (wifiReconnectTime < millis()) {
            Serial.println("Wifi need to reconnect");
            reconnectAttemptIndex += 1;
            WiFi.reconnect();
            wifiReconnectTime = millis() + wifiReconnectInterval;
        }
    }

    if (AppWiFi::isConnected()) {
        // Connection Succeed
        Serial.println("");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("DNS address: ");
        Serial.println(WiFi.dnsIP());
    }
}

void AppWiFi::connect() {
    // Wait for connection to establish
    Serial.print("Check WiFi connection...");
    if (!AppWiFi::isConnected()) {
        AppWiFi::reConnect();
    } else {
        WiFiClient client;
        const char *host = "google.com";
        if (!client.connect(host, 80)) {
            Serial.println("Google connection failed");
            client.stop();
            AppWiFi::reConnect();
            return;
        }

        // This will send the request to the server
        client.print(String("GET ") + "/ HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "Connection: close\r\n\r\n");

        unsigned long timeout = millis();
        while (client.available() == 0) {
            if (millis() - timeout > 5000) {
                Serial.println(">>> AppWiFi::connect Client Timeout !");
                client.stop();
                AppWiFi::reConnect();
                return;
            }
        }
        client.stop();

        Serial.println(" Already connected");
    }
}