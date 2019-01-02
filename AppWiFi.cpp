#include <Arduino.h>
#include <WiFi.h>

#include "AppWiFi.h"
#include "def.h"
#include "AppWiFiDef.h"

AppWiFi::AppWiFi() {}

AppWiFi::~AppWiFi() {}

void AppWiFi::connect() {
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
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PSWD);

    for (int loops = 10; loops > 0; loops--) {
        if (WiFi.isConnected()) {
            Serial.println("");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            Serial.print("DNS address: ");
            Serial.println(WiFi.dnsIP());
            break;
        } else {
            Serial.println(loops);
            delay(1000);
        }
    }
    if (!WiFi.isConnected()) {
        Serial.println("WiFi connect failed");
        delay(1000);
        ESP.restart();
    }
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