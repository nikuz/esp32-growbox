#include <Arduino.h>
#include <WiFi.h>
// HTTP implementation produce the "Task watchdog got triggered." error when Blynk response with 404 error
// #include <HTTPClient.h>

#include "Blynk.h"
#include "AppWiFi.h"
#include "AppStorage.h"
#include "def.h"
#include "BlynkDef.h"

static AppWiFi appWiFiClient;
static AppStorage appStorage;

#define TIMEOUT 2000
// 
const char* host = "blynk-cloud.com";
// const String host = "http://blynk-cloud.com/" + auth;
// pins
const String pinTemperature = "V0";
const String pinHumidity = "V1";
const String pinLight = "V2";
const String pinLightDayStart = "V6";
const String pinLightDayEnd = "V7";
const String pinVentilation = "V3";
const String pinVentilationTemperatureMax = "V8";
const String pinVersion = "V5";
const String pinPing = "V10";
const String pinRtcBattery = "V9";
const String pinOtaHost = "V20";
const String pinOtaBin = "V21";
const String pinOtaLastUpdateTime = "V22";
const String pinTerminal = "V30";
const String pinUptime = "V11";
const String pinRtcTemperature = "V12";
const String pinTime = "V13";

// cache
int fishIntCache = -1;
int temperatureCache = 0;
int humidityCache = 0;
int lightCache = 0;
int lightDayStartCache = 0;
int lightDayEndCache = 0;
int ventilationCache = 0;
int ventilationTemperatureMaxCache = 0;
int versionCache = 0;
int rtcBatteryCache = 0;
int rtcTemperatureCache = 0;
String fishStringCache = "fish";
String otaHostCache = "";
String otaBinCache = "";
String otaLastUpdateTimeCache = "";
String uptimeCache = "";

Blynk::Blynk() {}
Blynk::~Blynk() {}

// private 
String Blynk::getPinId(String pinId) {
    if (pinId == "temperature") return pinTemperature;
    if (pinId == "humidity") return pinHumidity;
    if (pinId == "light") return pinLight;
    if (pinId == "lightDayStart") return pinLightDayStart;
    if (pinId == "lightDayEnd") return pinLightDayEnd;
    if (pinId == "ventilation") return pinVentilation;
    if (pinId == "ventTempMax") return pinVentilationTemperatureMax;
    if (pinId == "version") return pinVersion;
    if (pinId == "ping") return pinPing;
    if (pinId == "rtcBattery") return pinRtcBattery;
    if (pinId == "otaHost") return pinOtaHost;
    if (pinId == "otaBin") return pinOtaBin;
    if (pinId == "otaLastUpdateTime") return pinOtaLastUpdateTime;
    if (pinId == "uptime") return pinUptime;
    if (pinId == "rtcTemperature") return pinRtcTemperature;
    if (pinId == "time") return pinTime;

    return "";
}

int& Blynk::getIntCacheValue(String pinId) {
    if (pinId == "temperature") return temperatureCache;
    if (pinId == "humidity") return humidityCache;
    if (pinId == "light") return lightCache;
    if (pinId == "lightDayStart") return lightDayStartCache;
    if (pinId == "lightDayEnd") return lightDayEndCache;
    if (pinId == "ventilation") return ventilationCache;
    if (pinId == "ventTempMax") return ventilationTemperatureMaxCache;
    if (pinId == "version") return versionCache;
    if (pinId == "rtcBattery") return rtcBatteryCache;
    if (pinId == "rtcTemperature") return rtcTemperatureCache;

    return fishIntCache;
}

String& Blynk::getStringCacheValue(String pinId) {
    if (pinId == "otaHost") return otaHostCache;
    if (pinId == "otaBin") return otaBinCache;
    if (pinId == "otaLastUpdateTime") return otaLastUpdateTimeCache;
    if (pinId == "uptime") return uptimeCache;
    
    return fishStringCache;
}

String Blynk::getPinUrl(String pinId) { return "/" + auth + "/get/" + pinId; }

String Blynk::getPinUpdateUrl(String pinId) { return "/" + auth + "/update/" + pinId; }

String Blynk::putPinGetUrl(String pinId) { return "/" + auth + "/update/" + pinId + "?value="; }

String Blynk::putPinUrl(String pinId, int value) { return this->putPinGetUrl(pinId) + String(value); }

String Blynk::putPinUrl(String pinId, String value) {
    String valueWithEncodedSpaces = value;
    valueWithEncodedSpaces.replace(" ", "%20");
    return this->putPinGetUrl(pinId) + valueWithEncodedSpaces;
}

// public
void Blynk::terminal(String value) {
    Serial.println(value);

    if (!appWiFiClient.isConnected()) {
        return;
    }

    String url = this->getPinUpdateUrl(pinTerminal);

    WiFiClient client;
    client.setTimeout(TIMEOUT);
    if (client.connect(host, 80)) {
        const String data = "[\"\\n" + value + "\"]";
        client.println("PUT " + url + " HTTP/1.1");
        client.println(String("Host: ") + host);
        client.println("Cache-Control: no-cache");
        client.println("Connection: close");
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(data.length());
        client.println();
        client.println(data);

        unsigned long requestTime = millis();
        while (client.available() == 0) {
            if (millis() - requestTime > TIMEOUT) {
                Serial.println(">>> Client Timeout!");
                break;
            }
        }

        while(client.available()) {
            String line = client.readStringUntil('\n');
            line.trim();
            if (line.startsWith("HTTP/1.1") && line.indexOf("200") < 0) {
                Serial.println("FAILED TERMINAL PUT: " + url);
            }
        }
    }
    client.stop();

    // HTTPClient http;
    // http.setTimeout(2000);
    // http.setReuse(false); // Connection: close
    // String requestUrl = this->getPinUpdateUrl(pinTerminal);
    // http.begin(requestUrl);
    // http.addHeader("Content-Type", "application/json");
    // int httpResponseCode = http.PUT("[\"\\n" + value + "\"]");
    // if (httpResponseCode != HTTP_CODE_OK) {
    //     Serial.print("FAILED TERMINAL PUT: " + String(httpResponseCode) + ": ");
    //     Serial.println(requestUrl);
    // }
    // http.end();
}

void Blynk::pingResponse() {
    if (!appWiFiClient.isConnected()) {
        return;
    }

    WiFiClient client;
    client.setTimeout(TIMEOUT);
    if (client.connect(host, 80)) {
        const String data = "{\"body\": \"PONG\"}";
        client.println("POST /" + auth + "/notify HTTP/1.1");
        client.println(String("Host: ") + host);
        client.println("Cache-Control: no-cache");
        client.println("Connection: close");
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(data.length());
        client.println();
        client.println(data);

        unsigned long requestTime = millis();
        while (client.available() == 0) {
            if (millis() - requestTime > TIMEOUT) {
                Serial.println(">>> Client Timeout!");
                break;
            }
        }

        while(client.available()) {
            String line = client.readStringUntil('\n');
            line.trim();
            if (line.startsWith("HTTP/1.1") && line.indexOf("200") < 0) {
                Serial.println("FAILED PONG");
            }
        }
    }
    client.stop();

    // HTTPClient http;
    // http.setTimeout(TIMEOUT);
    // http.begin(host + "/notify");
    // http.addHeader("Content-Type", "application/json");
    // int httpResponseCode = http.POST("{ \"body\": \"PONG\"}");
    // http.end();
}

String Blynk::getPinData(String pinId) {
    String blynkPin = this->getPinId(pinId);
    if (blynkPin == "") {
        return "";
    }
    if (!appWiFiClient.isConnected()) {
        return "";
    }

    const String url = this->getPinUrl(blynkPin);
    String response = "";
    
    WiFiClient client;
    client.setTimeout(TIMEOUT);
    if (client.connect(host, 80)) {
        client.println("GET " + url + " HTTP/1.1");
        client.println(String("Host: ") + host);
        client.println("Cache-Control: no-cache");
        client.println("Connection: close");
        client.println();

        unsigned long requestTime = millis();
        while (client.available() == 0) {
            if (millis() - requestTime > TIMEOUT) {
                Serial.println(">>> Client Timeout!");
                break;
            }
        }

        while(client.available()) {
            String line = client.readStringUntil('\n');
            line.trim();
            response = line;
            if (line.startsWith("HTTP/1.1") && line.indexOf("200") < 0) {
                Serial.println("FAILED GET: " + url);
                response = "";
            }
        }

        if (response != "") {
            response.replace("[\"", "");
            response.replace("\"]", "");
        }
    }
    client.stop();

    return response;

    // HTTPClient http;
    // http.setTimeout(2000);
    // http.setReuse(false); // Connection: close

    // const String pinUrl = this->getPinUrl(blynkPin);
    // String response = "";
    // http.begin(pinUrl);
    // int httpResponseCode = http.GET();
    // if (httpResponseCode == HTTP_CODE_OK) {
    //     response = http.getString();
    //     response.replace("[\"", "");
    //     response.replace("\"]", "");
    // } else {
    //     Serial.print("FAILED GET: " + String(httpResponseCode) + ": ");
    //     Serial.println(pinUrl);
    // }
    // http.end();

    return response;
}

void Blynk::getData(int& localVariable, const char* pinId, bool storePreferences) {
    const String pinData = this->getPinData(pinId);
    if (pinData == "") {
        return;
    }
    localVariable = pinData.toInt();
    if (storePreferences) {
        appStorage.putUInt(pinId, pinData.toInt());
    }
}

void Blynk::getData(String& localVariable, const char* pinId, bool storePreferences) {
    const String pinData = this->getPinData(pinId);
    if (pinData == "") {
        return;
    }
    localVariable = pinData;
    if (storePreferences) {
        appStorage.putString(pinId, pinData);
    }
}

void Blynk::postPinData(String pinId, String data) {
    if (!appWiFiClient.isConnected()) {
        return;
    }
    String url = this->putPinUrl(pinId, data);

    WiFiClient client;
    client.setTimeout(TIMEOUT);
    if (client.connect(host, 80)) {
        client.println("GET " + url + " HTTP/1.1");
        client.println(String("Host: ") + host);
        client.println("Cache-Control: no-cache");
        client.println("Connection: close");
        client.println();

        unsigned long requestTime = millis();
        while (client.available() == 0) {
            if (millis() - requestTime > TIMEOUT) {
                Serial.println(">>> Client Timeout!");
                break;
            }
        }

        while(client.available()) {
            String line = client.readStringUntil('\n');
            line.trim();
            if (line.startsWith("HTTP/1.1") && line.indexOf("200") < 0) {
                Serial.println("FAILED POST: " + url);
            }
        }
    }
    client.stop();

    // HTTPClient http;
    // http.setTimeout(2000);
    // http.setReuse(false); // Connection: close
    // http.begin(url);
    // int httpResponseCode = http.GET();
    // if (httpResponseCode != HTTP_CODE_OK) {
    //     Serial.print("FAILED POST: " + String(httpResponseCode) + ": ");
    //     Serial.println(url);
    // }
    // http.end();
}

void Blynk::postData(String pinId, int value) {
    String blynkPin = this->getPinId(pinId);
    if (blynkPin == "") {
        return;
    }

    int& cacheValue = getIntCacheValue(pinId);
    if (cacheValue != -1 || cacheValue != value) {
        this->postPinData(blynkPin, String(value));
        cacheValue = value;
    }
}

void Blynk::postData(String pinId, String value) {
    String blynkPin = this->getPinId(pinId);
    if (blynkPin == "") {
        return;
    }

    String& cacheValue = getStringCacheValue(pinId);
    if (cacheValue != "fish" || cacheValue != value) {
        this->postPinData(blynkPin, value);
    }
}
