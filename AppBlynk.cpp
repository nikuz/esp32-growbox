#include <Arduino.h>
#include <EspOta.h>

#include "def.h"

#ifdef DEBUG
#define BLYNK_DEBUG // Optional, this enables lots of prints
#define BLYNK_PRINT Serial
#endif
#define BLYNK_NO_BUILTIN   // Disable built-in analog & digital pin operations
#define BLYNK_NO_FLOAT     // Disable float operations
#define BLYNK_MSG_LIMIT 50

#include <BlynkSimpleEsp32.h>

#include "AppBlynkDef.h"
#include "AppBlynk.h"
#include "AppWiFi.h"
#include "AppStorage.h"
#include "Tools.h"
#include "Sensor.h"
#include "Relay.h"
#include "AppTime.h"
#include "Light.h"
#include "Watering.h"

// Blynk virtual pins
const int pinTemperature = V0;
const int pinHumidity = V1;
const int pinLight = V2;
const int pinLightIntensity = V25;
const int pinLightDayStart = V6;
const int pinLightDayEnd = V7;
const int pinVentilation = V3;
const int pinWind = V4;
const int pinVentilationTemperatureMax = V8;
const int pinVentilationHumidityMax = V14;
const int pinVersion = V5;
const int pinRtcBattery = V9;
const int pinOtaHost = V20;
const int pinOtaBin = V21;
const int pinOtaLastUpdateTime = V22;
const int pinUptime = V11;
const int pinRtcTemperature = V12;
const int pinHumidityWater = V15;
const int pinSoilMoisture1 = V16;
const int pinSoilMoisture2 = V17;
const int pinSoilMoisture3 = V18;
const int pinSoilMoisture4 = V19;
const int pinWSoilMstrMin = V23;
const int pinAutoWatering = V29;
const int pinWatering = V24;
const int pinWater = V28;
const int pinWaterLeakage = V32;
const int pinLightMaxInt = V26;
const int pinDoor = V27;
const int pinS1LstWtrng = V33;
const int pinS2LstWtrng = V34;
const int pinS3LstWtrng = V35;
const int pinS4LstWtrng = V36;
const int pinHLstWtrng = V37;
const int pinS1WtrngAuto = V38;
const int pinS2WtrngAuto = V39;
const int pinS3WtrngAuto = V40;
const int pinS4WtrngAuto = V41;
const int pinHWtrngAuto = V42;

// cache
int fishIntCache = -1;
int temperatureCache = 0;
int humidityCache = 0;
int lightCache = 0;
int lightIntensityCache = 0;
int lightMaxIntCache = 0;
int lightDayStartCache = 0;
int lightDayEndCache = 0;
int ventilationCache = 0;
int windCache = 0;
int ventilationTemperatureMaxCache = 0;
int ventilationHumidityMaxCache = 0;
int versionCache = 0;
int rtcBatteryCache = 0;
int rtcTemperatureCache = 0;
int humidityWaterCache = 0;
int soilMoistureCache1 = 0;
int soilMoistureCache2 = 0;
int soilMoistureCache3 = 0;
int soilMoistureCache4 = 0;
int wSoilMstrMinCache = 0;
int autoWateringCache = 0;
int wateringCache = 0;
int waterCache = 0;
int pinWaterLeakageCache = 0;
int doorCache = 0;
String fishStringCache = "fish";
String otaHostCache = "";
String otaBinCache = "";
String otaLastUpdateTimeCache = "";
String uptimeCache = "";
String s1LstWtrngCache = "";
String s2LstWtrngCache = "";
String s3LstWtrngCache = "";
String s4LstWtrngCache = "";
String hLstWtrngCache = "";

const unsigned long blynkConnectAttemptTime = 5UL * 1000UL;  // try to connect to blynk server only 5 seconds
bool blynkConnectAttemptFirstTime = true;
WidgetTerminal blynkTerminal(V30);

bool noHumidityWaterNotificationSent = false;
bool noWaterNotificationSent = false;
bool waterLeakageNotificationSent = false;

static BlynkIntVariable intVariables[15];
static BlynkStringVariable stringVariables[10];

AppBlynk::AppBlynk() {};

AppBlynk::~AppBlynk() {};

// private

int AppBlynk::getPinById(String pinId) {
    if (pinId == "temperature") return pinTemperature;
    if (pinId == "humidity") return pinHumidity;
    if (pinId == "light") return pinLight;
    if (pinId == "lightIntensity") return pinLightIntensity;
    if (pinId == "lightMaxInt") return pinLightMaxInt;
    if (pinId == "lightDayStart") return pinLightDayStart;
    if (pinId == "lightDayEnd") return pinLightDayEnd;
    if (pinId == "ventilation") return pinVentilation;
    if (pinId == "wind") return pinWind;
    if (pinId == "ventTempMax") return pinVentilationTemperatureMax;
    if (pinId == "ventHumMax") return pinVentilationHumidityMax;
    if (pinId == "version") return pinVersion;
    if (pinId == "rtcBattery") return pinRtcBattery;
    if (pinId == "otaHost") return pinOtaHost;
    if (pinId == "otaBin") return pinOtaBin;
    if (pinId == "otaLastUpdateTime") return pinOtaLastUpdateTime;
    if (pinId == "uptime") return pinUptime;
    if (pinId == "rtcTemperature") return pinRtcTemperature;
    if (pinId == "humidityWater") return pinHumidityWater;
    if (pinId == "soilMoisture1") return pinSoilMoisture1;
    if (pinId == "soilMoisture2") return pinSoilMoisture2;
    if (pinId == "soilMoisture3") return pinSoilMoisture3;
    if (pinId == "soilMoisture4") return pinSoilMoisture4;
    if (pinId == "wSoilMstrMin") return pinWSoilMstrMin;
    if (pinId == "water") return pinWater;
    if (pinId == "autoWatering") return pinAutoWatering;
    if (pinId == "watering") return pinWatering;
    if (pinId == "waterLeakage") return pinWaterLeakage;
    if (pinId == "door") return pinDoor;
    if (pinId == "s1LstWtrng") return pinS1LstWtrng;
    if (pinId == "s2LstWtrng") return pinS2LstWtrng;
    if (pinId == "s3LstWtrng") return pinS3LstWtrng;
    if (pinId == "s4LstWtrng") return pinS4LstWtrng;
    if (pinId == "hLstWtrng") return pinHLstWtrng;
    if (pinId == "s1WtrngAuto") return pinS1WtrngAuto;
    if (pinId == "s2WtrngAuto") return pinS2WtrngAuto;
    if (pinId == "s3WtrngAuto") return pinS3WtrngAuto;
    if (pinId == "s4WtrngAuto") return pinS4WtrngAuto;
    if (pinId == "hWtrngAuto") return pinHWtrngAuto;

    return -1;
}

int &AppBlynk::getIntCacheValue(String pinId) {
    if (pinId == "temperature") return temperatureCache;
    if (pinId == "humidity") return humidityCache;
    if (pinId == "light") return lightCache;
    if (pinId == "lightIntensity") return lightIntensityCache;
    if (pinId == "lightDayStart") return lightDayStartCache;
    if (pinId == "lightDayEnd") return lightDayEndCache;
    if (pinId == "ventilation") return ventilationCache;
    if (pinId == "wind") return windCache;
    if (pinId == "ventTempMax") return ventilationTemperatureMaxCache;
    if (pinId == "ventHumMax") return ventilationHumidityMaxCache;
    if (pinId == "version") return versionCache;
    if (pinId == "rtcBattery") return rtcBatteryCache;
    if (pinId == "rtcTemperature") return rtcTemperatureCache;
    if (pinId == "humidityWater") return humidityWaterCache;
    if (pinId == "soilMoisture1") return soilMoistureCache1;
    if (pinId == "soilMoisture2") return soilMoistureCache2;
    if (pinId == "soilMoisture3") return soilMoistureCache3;
    if (pinId == "soilMoisture4") return soilMoistureCache4;
    if (pinId == "wSoilMstrMin") return wSoilMstrMinCache;
    if (pinId == "autoWatering") return autoWateringCache;
    if (pinId == "watering") return wateringCache;
    if (pinId == "water") return waterCache;
    if (pinId == "waterLeakage") return pinWaterLeakageCache;
    if (pinId == "door") return doorCache;

    return fishIntCache;
}

String &AppBlynk::getStringCacheValue(String pinId) {
    if (pinId == "otaHost") return otaHostCache;
    if (pinId == "otaBin") return otaBinCache;
    if (pinId == "otaLastUpdateTime") return otaLastUpdateTimeCache;
    if (pinId == "uptime") return uptimeCache;
    if (pinId == "s1LstWtrng") return s1LstWtrngCache;
    if (pinId == "s2LstWtrng") return s2LstWtrngCache;
    if (pinId == "s3LstWtrng") return s3LstWtrngCache;
    if (pinId == "s4LstWtrng") return s4LstWtrngCache;
    if (pinId == "hLstWtrng") return hLstWtrngCache;

    return fishStringCache;
}

int &AppBlynk::getIntVariable(const char *pin) {
    const int intVarsLen = *(&intVariables + 1) - intVariables;
    for (int i = 0; i < intVarsLen; i++) {
        if (intVariables[i].pin == pin) {
            return *intVariables[i].var;
        }
    }

    return fishIntCache;
}

String &AppBlynk::getStringVariable(const char *pin) {
    const int stringVarsLen = *(&stringVariables + 1) - stringVariables;
    for (int i = 0; i < stringVarsLen; i++) {
        if (stringVariables[i].pin == pin) {
            return *stringVariables[i].var;
        }
    }

    return fishStringCache;
}

void AppBlynk::sync() { // every 60 sec
    if (!AppWiFi::isConnected() || !Blynk.connected() || Tools::millisOverflowIsClose()) {
        return;
    }

    const char *otaHostPin = "otaHost";
    String &otaHostVariable = AppBlynk::getStringVariable(otaHostPin);
    AppBlynk::postData("otaHost", otaHostVariable);

    const char *otaBinPin = "otaBin";
    String &otaBinVariable = AppBlynk::getStringVariable(otaBinPin);
    AppBlynk::postData("otaBin", otaBinVariable);

    AppBlynk::postData("otaLastUpdateTime", EspOta::getUpdateTime());
    AppBlynk::postData("uptime", String(Tools::getUptime()));
    AppBlynk::postData("version", VERSION);
}

void AppBlynk::syncHighFreq1() { // every 3 sec
    if (!AppWiFi::isConnected() || !Blynk.connected() || Tools::millisOverflowIsClose()) {
        return;
    }

    AppBlynk::postData("temperature", Sensor::temperatureGet());
    AppBlynk::postData("humidity", Sensor::humidityGet());
    AppBlynk::postData("light", Relay::isLightOn() ? 255 : 0);
    AppBlynk::postData("ventilation", Relay::isVentilationOn() ? 255 : 0);
    AppBlynk::postData("wind", Relay::isWindOn() ? 255 : 0);
    AppBlynk::postData("rtcBattery", AppTime::RTCBattery() ? 255 : 0);
    AppBlynk::postData("rtcTemperature", AppTime::RTCGetTemperature());
    AppBlynk::postData("humidityWater", Sensor::humidityHasWater() ? 255 : 0);

#if PRODUCTION
    bool humidityHasWater = Sensor::humidityHasWater();
    if (!humidityHasWater && !noHumidityWaterNotificationSent) {
        Blynk.notify("Humidity has no water");
        noHumidityWaterNotificationSent = true;
    } else if (humidityHasWater && noHumidityWaterNotificationSent) {
        noHumidityWaterNotificationSent = false;
    }

    bool wateringHasWater = Sensor::wateringHasWater();
    if (!wateringHasWater && !noWaterNotificationSent) {
        Blynk.notify("Watering has no water");
        noWaterNotificationSent = true;
    } else if (wateringHasWater && noWaterNotificationSent) {
        noWaterNotificationSent = false;
    }

    bool waterLeakage = Sensor::waterLeakageDetected();
    if (waterLeakage && !waterLeakageNotificationSent) {
        Blynk.notify("Water leakage detected!!!");
        waterLeakageNotificationSent = true;
    } else if (!waterLeakage && waterLeakageNotificationSent) {
        waterLeakageNotificationSent = false;
    }
#endif
}

void AppBlynk::syncHighFreq2() { // every 4 sec
    if (!AppWiFi::isConnected() || !Blynk.connected() || Tools::millisOverflowIsClose()) {
        return;
    }

    AppBlynk::postData("soilMoisture1", Sensor::getSoilMoisture(SOIL_SENSOR_1));
    AppBlynk::postData("soilMoisture2", Sensor::getSoilMoisture(SOIL_SENSOR_2));
    AppBlynk::postData("soilMoisture3", Sensor::getSoilMoisture(SOIL_SENSOR_3));
    AppBlynk::postData("soilMoisture4", Sensor::getSoilMoisture(SOIL_SENSOR_4));
    AppBlynk::postData("watering", Relay::isWateringOn() ? 255 : 0);
    AppBlynk::postData("water", Sensor::wateringHasWater() ? 255 : 0);
    AppBlynk::postData("waterLeakage", Sensor::waterLeakageDetected() ? 255 : 0);
    AppBlynk::postData("lightIntensity", Light::intensity());
    AppBlynk::postData("door", Sensor::doorIsOpen() ? 0 : 255);
}

void AppBlynk::syncHighFreq3() { // every 5 sec
    if (!AppWiFi::isConnected() || !Blynk.connected() || Tools::millisOverflowIsClose()) {
        return;
    }

    AppBlynk::postData("s1LstWtrng", Watering::getStringVariable("s1LstWtrng"));
    AppBlynk::postData("s2LstWtrng", Watering::getStringVariable("s2LstWtrng"));
    AppBlynk::postData("s3LstWtrng", Watering::getStringVariable("s3LstWtrng"));
    AppBlynk::postData("s4LstWtrng", Watering::getStringVariable("s4LstWtrng"));
    AppBlynk::postData("hLstWtrng", Watering::getStringVariable("hLstWtrng"));
}

void writeHandler(const char *pin, int value, bool store) {
    int &variable = AppBlynk::getIntVariable(pin);
    AppBlynk::getData(variable, pin, value, store);
}

void writeHandler(const char *pin, String value, bool store) {
    String &variable = AppBlynk::getStringVariable(pin);
    AppBlynk::getData(variable, pin, value, store);
}

BLYNK_WRITE(V6) { // lightDayStart
    writeHandler("lightDayStart", param.asInt(), true);
}
BLYNK_WRITE(V7) { // lightDayEnd
    writeHandler("lightDayEnd", param.asInt(), true);
}
BLYNK_WRITE(V8) { // ventTempMax
    writeHandler("ventTempMax", param.asInt(), true);
}
BLYNK_WRITE(V14) { // ventHumMax
    writeHandler("ventHumMax", param.asInt(), true);
}
BLYNK_WRITE(V20) { // otaHost
    writeHandler("otaHost", param.asStr(), true);
}
BLYNK_WRITE(V21) { // otaBin
    writeHandler("otaBin", param.asStr(), true);
}
BLYNK_WRITE(V23) { // wSoilMstrMin
    writeHandler("wSoilMstrMin", param.asInt(), true);
}
BLYNK_WRITE(V26) { // lightMaxInt
    writeHandler("lightMaxInt", param.asInt(), true);
}
BLYNK_WRITE(V29) { // autoWatering
    int value = param.asInt();
    if (value == 0) {
        Watering::stop();
    }
    writeHandler("autoWatering", value, false);
}
BLYNK_WRITE(V38) {
    int value = param.asInt();
    if (value == 0) {
        Watering::stop();
    }
    writeHandler("s1WtrngAuto", value, false);
}

BLYNK_WRITE(V39) {
    int value = param.asInt();
    if (value == 0) {
        Watering::stop();
    }
    writeHandler("s2WtrngAuto", value, false);
}

BLYNK_WRITE(V40) {
    int value = param.asInt();
    if (value == 0) {
        Watering::stop();
    }
    writeHandler("s3WtrngAuto", value, false);
}

BLYNK_WRITE(V41) {
    int value = param.asInt();
    if (value == 0) {
        Watering::stop();
    }
    writeHandler("s4WtrngAuto", value, false);
}

BLYNK_WRITE(V42) {
    int value = param.asInt();
    if (value == 0) {
        Watering::stop();
    }
    writeHandler("hWtrngAuto", value, false);
}
BLYNK_WRITE(V10) { // ping
    if (param.asInt() == 1) {
        Blynk.notify("PONG");
        Blynk.virtualWrite(V10, 0);
    }
}
BLYNK_WRITE(V13) { // get time
    if (param.asInt() == 1) {
        AppTime::print();
        Blynk.virtualWrite(V13, 0);
    }
}
BLYNK_WRITE(V31) { // restart
    if (param.asInt() == 1) {
        Blynk.virtualWrite(V31, 0);
        delay(2000);
        ESP.restart();
    }
}

BLYNK_CONNECTED() {
    Blynk.syncAll();
}

// public

void AppBlynk::setVariable(int *var, const char *pin, bool store) {
    int varsCount = *(&intVariables + 1) - intVariables;
    for (int i = 0; i < varsCount; i++) {
        if (!intVariables[i].pin) {
            intVariables[i] = BlynkIntVariable(var, pin, store);
            break;
        }
    }
}

void AppBlynk::setVariable(String *var, const char *pin, bool store) {
    int varsCount = *(&stringVariables + 1) - stringVariables;
    for (int i = 0; i < varsCount; i++) {
        if (!stringVariables[i].pin) {
            stringVariables[i] = BlynkStringVariable(var, pin, store);
            break;
        }
    }
}

void AppBlynk::checkConnect() {
    if (!blynkConnectAttemptFirstTime && Tools::millisOverflowIsClose()) {
        return;
    }
    if (AppWiFi::isConnected() && !Blynk.connected()) {
        unsigned long startConnecting = millis();
        while (!Blynk.connected()) {
            Blynk.connect();
            if (millis() > startConnecting + blynkConnectAttemptTime) {
                Serial.println("Unable to connect to Blynk server.\n");
                break;
            }
        }
        if (Blynk.connected() && blynkConnectAttemptFirstTime) {
            blynkTerminal.clear();
            sync();
        }
        blynkConnectAttemptFirstTime = false;
    }
}

void AppBlynk::initiate() {
    Blynk.config(blynkAuth, blynkDomain, blynkPort);
    AppBlynk::checkConnect();
}

void AppBlynk::run() {
    if (Blynk.connected()) {
        Blynk.run();
    }
}

void AppBlynk::getData(int &localVariable, const char *pinId, int pinData, const bool storePreferences) {
    int blynkPin = AppBlynk::getPinById(pinId);
    if (localVariable == -1 || blynkPin == -1) {
        return;
    }
    if (pinData != localVariable) {
        localVariable = pinData;
        if (storePreferences) {
            AppStorage::putUInt(pinId, pinData);
        }
    }
}

void AppBlynk::getData(String &localVariable, const char *pinId, String pinData, const bool storePreferences) {
    int blynkPin = AppBlynk::getPinById(pinId);
    if (localVariable == "fish" || blynkPin == -1) {
        return;
    }
    if (pinData != localVariable) {
        localVariable = pinData;
        if (storePreferences) {
            AppStorage::putString(pinId, pinData);
        }
    }
}

void AppBlynk::postData(String pinId, int value) {
    int blynkPin = AppBlynk::getPinById(pinId);
    if (blynkPin == -1) {
        return;
    }
    int &cacheValue = AppBlynk::getIntCacheValue(pinId);
    if (cacheValue != -1 || cacheValue != value) { // post data also if cache not applied for pin
        if (Blynk.connected()) {
            Blynk.virtualWrite(blynkPin, value);
        }
        cacheValue = value;
    }
}

void AppBlynk::postData(String pinId, String value) {
    int blynkPin = AppBlynk::getPinById(pinId);
    if (blynkPin == -1) {
        return;
    }
    String &cacheValue = AppBlynk::getStringCacheValue(pinId);
    if (cacheValue != "fish" || cacheValue != value) { // post data also if cache not applied for pin
        if (Blynk.connected()) {
            Blynk.virtualWrite(blynkPin, value);
        }
        cacheValue = value;
    }
}

void AppBlynk::postDataNoCache(String pinId, int value) {
    int blynkPin = AppBlynk::getPinById(pinId);
    if (blynkPin == -1) {
        return;
    }
    if (Blynk.connected()) {
        Blynk.virtualWrite(blynkPin, value);
    }
}

void AppBlynk::postDataNoCache(String pinId, String value) {
    int blynkPin = AppBlynk::getPinById(pinId);
    if (blynkPin == -1) {
        return;
    }
    if (Blynk.connected()) {
        Blynk.virtualWrite(blynkPin, value);
    }
}

void AppBlynk::print(String value) {
    Serial.print(value);
    if (AppWiFi::isConnected() && Blynk.connected() && !Tools::millisOverflowIsClose()) {
        blynkTerminal.print(value);
    }
}

void AppBlynk::print(char *value) {
    Serial.print(value);
    if (AppWiFi::isConnected() && Blynk.connected() && !Tools::millisOverflowIsClose()) {
        blynkTerminal.print(value);
    }
}

void AppBlynk::print(int value) {
    Serial.print(value);
    if (AppWiFi::isConnected() && Blynk.connected() && !Tools::millisOverflowIsClose()) {
        blynkTerminal.print(value);
    }
}

void AppBlynk::print(double value) {
    Serial.print(value);
    if (AppWiFi::isConnected() && Blynk.connected() && !Tools::millisOverflowIsClose()) {
        blynkTerminal.print(value);
    }
}

void AppBlynk::println(String value) {
    Serial.println(value);
    if (AppWiFi::isConnected() && Blynk.connected() && !Tools::millisOverflowIsClose()) {
        blynkTerminal.print(value);
        blynkTerminal.flush();
    }
}

void AppBlynk::println(char *value) {
    Serial.println(value);
    if (AppWiFi::isConnected() && Blynk.connected() && !Tools::millisOverflowIsClose()) {
        blynkTerminal.println(value);
        blynkTerminal.flush();
    }
}

void AppBlynk::println(int value) {
    Serial.println(value);
    if (AppWiFi::isConnected() && Blynk.connected() && !Tools::millisOverflowIsClose()) {
        blynkTerminal.println(value);
        blynkTerminal.flush();
    }
}

void AppBlynk::println(double value) {
    Serial.println(value);
    if (AppWiFi::isConnected() && Blynk.connected() && !Tools::millisOverflowIsClose()) {
        blynkTerminal.println(value);
        blynkTerminal.flush();
    }
}
