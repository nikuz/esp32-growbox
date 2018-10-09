#include <Arduino.h>
#include <HTTPClient.h>

#include "Blynk.h"
#include "AppWiFi.h"
#include "def.h"
#include "BlynkDef.h"

static AppWiFi appWiFiClient;

// 
const String host = "http://blynk-cloud.com/" + auth;
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
    if (pinId == "ventilationTemperatureMax") return pinVentilationTemperatureMax;
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

String Blynk::getPinUrl(String pinId) { return host + "/get/" + pinId; }

String Blynk::getPinUpdateUrl(String pinId) { return host + "/update/" + pinId; }

String Blynk::putPinGetUrl(String pinId) { return host + "/update/" + pinId + "?value="; }

String Blynk::putPinUrl(String pinId, int value) { return this->putPinGetUrl(pinId) + String(value); }

String Blynk::putPinUrl(String pinId, String value) {
    String valueWithEncodedSpaces = value;
    valueWithEncodedSpaces.replace(" ", "%20");
    return this->putPinGetUrl(pinId) + valueWithEncodedSpaces;
}

void Blynk::postPinData(String pinId, String data) {
    if (!appWiFiClient.isConnected()) {
        return;
    }
    String url = this->putPinUrl(pinId, data);

    HTTPClient http;
    http.setTimeout(2000);
    http.setReuse(false); // Connection: close
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode != HTTP_CODE_OK) {
        Serial.print("FAILED POST: " + String(httpResponseCode) + ": ");
        Serial.println(url);
    }
    http.end();
}

// public
void Blynk::terminal(String value) {
    Serial.println(value);
    if (!appWiFiClient.isConnected()) {
        return;
    }
    HTTPClient http;
    http.setTimeout(2000);
    http.setReuse(false); // Connection: close
    String requestUrl = this->getPinUpdateUrl(pinTerminal);
    http.begin(requestUrl);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.PUT("[\"\\n" + value + "\"]");
    if (httpResponseCode != HTTP_CODE_OK) {
        Serial.print("FAILED TERMINAL PUT: " + String(httpResponseCode) + ": ");
        Serial.println(requestUrl);
    }
    http.end();
}

void Blynk::pingResponse() {
    if (!appWiFiClient.isConnected()) {
        return;
    }
    HTTPClient http;
    http.setTimeout(2000);
    http.begin(host + "/notify");
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST("{ \"body\": \"PONG\"}");
    http.end();
}

String Blynk::getData(String pinId) {
    String blynkPin = this->getPinId(pinId);
    if (blynkPin == "") {
        return "";
    }
    if (!appWiFiClient.isConnected()) {
        return "";
    }
    HTTPClient http;
    http.setTimeout(2000);
    http.setReuse(false); // Connection: close

    const String pinUrl = this->getPinUrl(blynkPin);
    String response = "";
    http.begin(pinUrl);
    int httpResponseCode = http.GET();
    if (httpResponseCode == HTTP_CODE_OK) {
        response = http.getString();
        response.replace("[\"", "");
        response.replace("\"]", "");
    } else {
        Serial.print("FAILED GET: " + String(httpResponseCode) + ": ");
        Serial.println(pinUrl);
    }
    http.end();
    Serial.println(http.connected());

    return response;
}

void Blynk::postData(String pinId, int value) {
    String blynkPin = this->getPinId(pinId);
    if (blynkPin == "") {
        return;
    }

    this->postPinData(blynkPin, String(value));
}

void Blynk::postData(String pinId, String value) {
    String blynkPin = this->getPinId(pinId);
    if (blynkPin == "") {
        return;
    }

    this->postPinData(blynkPin, value);
}
