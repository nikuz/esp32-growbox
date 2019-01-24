#include <Arduino.h>
#include <WiFi.h>

#include "def.h"
#include "AppWiFiDef.h"
#include "AppWiFi.h"

AppWiFi::AppWiFi() {}

AppWiFi::~AppWiFi() {}

void AppWiFi::connect() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PSWD);

    for (int loops = 10; loops > 0; loops--) {
        if (WiFi.isConnected()) {
            DEBUG_PRINTLN("");
            DEBUG_PRINT("IP address: ");
            DEBUG_PRINTLN(WiFi.localIP());
            DEBUG_PRINT("DNS address: ");
            DEBUG_PRINTLN(WiFi.dnsIP());
            break;
        } else {
            DEBUG_PRINTLN(loops);
            delay(1000);
        }
    }
    if (!WiFi.isConnected()) {
        DEBUG_PRINTLN("WiFi connect failed");
    }
}

bool AppWiFi::isConnected() {
    return WiFi.isConnected();
}