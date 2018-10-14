// http responses was reworked on WiFiClient because HTTP implementation produce 
// the "Task watchdog got triggered." error when Blynk response with 404 error

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#include "Blynk.h"
#include "AppWiFi.h"
#include "AppStorage.h"
#include "def.h"
#include "BlynkDef.h"

static AppWiFi appWiFiClient;
static AppStorage appStorage;

#define TIMEOUT 2000
const char* host = "blynk-cloud.com";

// Blynk virtual pins
const int pinTemperature = 0;
const int pinHumidity = 1;
const int pinLight = 2;
const int pinLightDayStart = 6;
const int pinLightDayEnd = 7;
const int pinVentilation = 3;
const int pinVentilationTemperatureMax = 8;
const int pinVentilationHumidityMax = 14;
const int pinVersion = 5;
const int pinPing = 10;
const int pinRtcBattery = 9;
const int pinOtaHost = 20;
const int pinOtaBin = 21;
const int pinOtaLastUpdateTime = 22;
const int pinTerminal = 30;
const int pinUptime = 11;
const int pinRtcTemperature = 12;
const int pinTime = 13;
const int pinRestart = 31;
const int pinHumidityWater = 15;
const int pinSoilMoisture1 = 16;
const int pinSoilMoisture2 = 17;

// cache
int fishIntCache = -1;
int temperatureCache = 0;
int humidityCache = 0;
int lightCache = 0;
int lightDayStartCache = 0;
int lightDayEndCache = 0;
int ventilationCache = 0;
int ventilationTemperatureMaxCache = 0;
int ventilationHumidityMaxCache = 0;
int versionCache = 0;
int rtcBatteryCache = 0;
int rtcTemperatureCache = 0;
int humidityWaterCache = 0;
int soilMoistureCache1 = 0;
int soilMoistureCache2 = 0;
String fishStringCache = "fish";
String otaHostCache = "";
String otaBinCache = "";
String otaLastUpdateTimeCache = "";
String uptimeCache = "";

static BlynkIntVariable intVariables[10];
static BlynkStringVariable stringVariables[10];

Blynk::Blynk() {}
Blynk::~Blynk() {}

// private 
int Blynk::getPinById(String pinId) {
    if (pinId == "temperature") return pinTemperature;
    if (pinId == "humidity") return pinHumidity;
    if (pinId == "light") return pinLight;
    if (pinId == "lightDayStart") return pinLightDayStart;
    if (pinId == "lightDayEnd") return pinLightDayEnd;
    if (pinId == "ventilation") return pinVentilation;
    if (pinId == "ventTempMax") return pinVentilationTemperatureMax;
    if (pinId == "ventHumMax") return pinVentilationHumidityMax;
    if (pinId == "version") return pinVersion;
    if (pinId == "ping") return pinPing;
    if (pinId == "rtcBattery") return pinRtcBattery;
    if (pinId == "otaHost") return pinOtaHost;
    if (pinId == "otaBin") return pinOtaBin;
    if (pinId == "otaLastUpdateTime") return pinOtaLastUpdateTime;
    if (pinId == "uptime") return pinUptime;
    if (pinId == "rtcTemperature") return pinRtcTemperature;
    if (pinId == "time") return pinTime;
    if (pinId == "restart") return pinRestart;
    if (pinId == "humidityWater") return pinHumidityWater;
    if (pinId == "soilMoisture1") return pinSoilMoisture1;
    if (pinId == "soilMoisture2") return pinSoilMoisture2;

    return -1;
}

char* Blynk::getIdByPin(int pin) {
    if (pin == pinLightDayStart) return "lightDayStart";
    if (pin == pinLightDayEnd) return "lightDayEnd";
    if (pin == pinVentilationTemperatureMax) return "ventTempMax";
    if (pin == pinVentilationHumidityMax) return "ventHumMax";
    if (pin == pinPing) return "ping";
    if (pin == pinTime) return "time";
    if (pin == pinOtaHost) return "otaHost";
    if (pin == pinOtaBin) return "otaBin";
    if (pin == pinRestart) return "restart";

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
    if (pinId == "ventHumMax") return ventilationHumidityMaxCache;
    if (pinId == "version") return versionCache;
    if (pinId == "rtcBattery") return rtcBatteryCache;
    if (pinId == "rtcTemperature") return rtcTemperatureCache;
    if (pinId == "humidityWater") return humidityWaterCache;
    if (pinId == "soilMoisture1") return soilMoistureCache1;
    if (pinId == "soilMoisture2") return soilMoistureCache2;

    return fishIntCache;
}

String& Blynk::getStringCacheValue(String pinId) {
    if (pinId == "otaHost") return otaHostCache;
    if (pinId == "otaBin") return otaBinCache;
    if (pinId == "otaLastUpdateTime") return otaLastUpdateTimeCache;
    if (pinId == "uptime") return uptimeCache;
    
    return fishStringCache;
}

String Blynk::getPinUrl(int pinId) { return "/" + auth + "/get/V" + pinId; }

String Blynk::getPinUpdateUrl(int pinId) { return "/" + auth + "/update/V" + pinId; }

String Blynk::putPinGetUrl(int pinId) { return "/" + auth + "/update/V" + pinId + "?value="; }

String Blynk::putPinUrl(int pinId, int value) { return this->putPinGetUrl(pinId) + String(value); }

String Blynk::putPinUrl(int pinId, String value) {
    String valueWithEncodedSpaces = value;
    valueWithEncodedSpaces.replace(" ", "%20");
    return this->putPinGetUrl(pinId) + valueWithEncodedSpaces;
}

// public
void Blynk::setVariable(int* var, const char* pin, bool store) {
    int urlCounts = *(&intVariables + 1) - intVariables;
    for (int i = 0; i < urlCounts; i++) {
        if (!intVariables[i].pin) {
            intVariables[i] = BlynkIntVariable(var, pin, store);
            break;
        }
    }
}

void Blynk::setVariable(String* var, const char* pin, bool store) {
    int urlCounts = *(&stringVariables + 1) - stringVariables;
    for (int i = 0; i < urlCounts; i++) {
        if (!stringVariables[i].pin) {
            stringVariables[i] = BlynkStringVariable(var, pin, store);
            break;
        }
    }
}

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
                Serial.println(">>> Blynk::terminal Client Timeout!");
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
}

void Blynk::notification(String message) {
    if (!appWiFiClient.isConnected()) {
        return;
    }

    WiFiClient client;
    client.setTimeout(TIMEOUT);
    if (client.connect(host, 80)) {
        #if PRODUCTION
        const String devMarker = "";
        #else
        const String devMarker = "-DEV";
        #endif
        const String data = "{\"body\": \"GROWBOX" + devMarker + ": " + message + "\"}";
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
                Serial.println(">>> Blynk::notification Client Timeout!");
                break;
            }
        }

        while(client.available()) {
            String line = client.readStringUntil('\n');
            line.trim();
            if (line.startsWith("HTTP/1.1") && line.indexOf("200") < 0) {
                Serial.println("FAILED BLYNK NOTIFICATION");
            }
        }
    }
    client.stop();
}

String Blynk::getPinData(String pinId) {
    int blynkPin = this->getPinById(pinId);
    if (blynkPin == -1) {
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
                Serial.println(">>> Blynk::getPinData Client Timeout!");
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
}

void Blynk::getData(int& localVariable, const char* pinId, bool storePreferences) {
    const String pinData = this->getPinData(pinId);
    if (pinData == "") {
        return;
    }
    const int newData = pinData.toInt();
    if (newData != localVariable) {
        localVariable = pinData.toInt();
        if (storePreferences) {
            appStorage.putUInt(pinId, pinData.toInt());
        }
    }
}

void Blynk::getData(String& localVariable, const char* pinId, bool storePreferences) {
    const String pinData = this->getPinData(pinId);
    if (pinData == "") {
        return;
    }
    if (pinData != localVariable) {
        localVariable = pinData;
        if (storePreferences) {
            appStorage.putString(pinId, pinData);
        }
    }
}

void Blynk::postPinData(int pinId, String data) {
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
                Serial.println(">>> Blynk::postPinData Client Timeout!");
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
}

void Blynk::postData(String pinId, int value) {
    int blynkPin = this->getPinById(pinId);
    if (blynkPin == -1) {
        return;
    }

    int& cacheValue = getIntCacheValue(pinId);
    if (cacheValue != -1 || cacheValue != value) { // post data also if cache not applied for pin
        this->postPinData(blynkPin, String(value));
        cacheValue = value;
    }
}

void Blynk::postData(String pinId, String value) {
    int blynkPin = this->getPinById(pinId);
    if (blynkPin == -1) {
        return;
    }

    String& cacheValue = getStringCacheValue(pinId);
    if (cacheValue != "fish" || cacheValue != value) { // post data also if cache not applied for pin
        this->postPinData(blynkPin, value);
        cacheValue = value;
    }
}

void Blynk::getProject() {
    WiFiClient client;
    client.setTimeout(TIMEOUT);
    if (client.connect(host, 80)) {
        client.println("GET /" + auth + "/project HTTP/1.1");
        client.println(String("Host: ") + host);
        client.println("Cache-Control: no-cache");
        client.println("Connection: close");
        client.println("Content-Type: application/json");
        client.println();

        unsigned long requestTime = millis();
        while (client.available() == 0) {
            if (millis() - requestTime > TIMEOUT) {
                Serial.println(">>> Blynk::getProject Client Timeout!");
                break;
            }
        }

        String response = "";
        while(client.available()) {
            String line = client.readStringUntil('\n');
            line.trim();

            response = line;
            if (line.startsWith("HTTP/1.1") && line.indexOf("200") < 0) {
                Serial.println("FAILED GET: /project");
                response = "";
            }
        }

        if (response != "") {
            // size of jsonBuffer was calculated here https://arduinojson.org/v5/assistant/
            // and defined with a margin to future
            DynamicJsonBuffer jsonBuffer(20000);
            JsonObject& root = jsonBuffer.parseObject(response);
            if (root.success()) {
                JsonArray& widgets = root["widgets"];
                if (widgets.success()) {
                    for (JsonObject& elem : widgets) {
                        const int pin = elem["pin"];
                        const String value = elem["value"];
                        if (pin != 0 && value != "") {
                            const char* pinId = this->getIdByPin(pin);
                            if (pinId != "") {
                                const int intVarsLen = *(&intVariables + 1) - intVariables;
                                const int intValue = value.toInt();
                                for (int i = 0; i < intVarsLen; i++) {
                                    if (intVariables[i].pin && intVariables[i].pin == pinId && *intVariables[i].var != intValue) {
                                        *intVariables[i].var = intValue;
                                        if (intVariables[i].store) {
                                            appStorage.putUInt(pinId, intValue);
                                        }
                                    }
                                }

                                const int stringVarsLen = *(&stringVariables + 1) - stringVariables;
                                for (int i = 0; i < stringVarsLen; i++) {
                                    if (stringVariables[i].pin && stringVariables[i].pin == pinId && *stringVariables[i].var != value) {
                                        *stringVariables[i].var = value;
                                        if (stringVariables[i].store) {
                                            appStorage.putString(pinId, value);
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else {
                    Serial.println("GET PROJECT: no widgets");
                }
            } else {
                Serial.println("GET PROJECT: parseObject() failed");
            }
        }
    }
    client.stop();
}