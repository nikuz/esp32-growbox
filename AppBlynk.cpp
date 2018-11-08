
#include <Arduino.h>
#include <EspOta.h>

#define BLYNK_PRINT Serial

#include <BlynkSimpleEsp32.h>

#include "def.h"
#include "AppBlynkDef.h"
#include "AppBlynk.h"
#include "AppWiFi.h"
#include "AppStorage.h"
#include "Tools.h"
#include "Sensor.h"
#include "Relay.h"
#include "AppTime.h"

// Blynk virtual pins
const int pinTemperature = V0;
const int pinHumidity = V1;
const int pinLight = V2;
const int pinLightDayStart = V6;
const int pinLightDayEnd = V7;
const int pinVentilation = V3;
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
int soilMoistureCache3 = 0;
int soilMoistureCache4 = 0;
String fishStringCache = "fish";
String otaHostCache = "";
String otaBinCache = "";
String otaLastUpdateTimeCache = "";
String uptimeCache = "";

const int blynkSyncInterval = 1000l * 60L;  // sync blynk low frew state every 60 seconds
BlynkTimer blynkSyncTimer;
const int blynkSyncHighFreqInterval = 1000l * 2L;  // sync blynk high freq state every 2 seconds
BlynkTimer blynkSyncHighFreqTimer;
WidgetTerminal blynkTerminal(V30);

bool noHumidityWaterNotificationSent = false;

static BlynkIntVariable intVariables[10];
static BlynkStringVariable stringVariables[10];

AppBlynk::AppBlynk() {};

AppBlynk::~AppBlynk() {};

// private

int AppBlynk::getPinById(String pinId) {
    if (pinId == "temperature") return pinTemperature;
    if (pinId == "humidity") return pinHumidity;
    if (pinId == "light") return pinLight;
    if (pinId == "lightDayStart") return pinLightDayStart;
    if (pinId == "lightDayEnd") return pinLightDayEnd;
    if (pinId == "ventilation") return pinVentilation;
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

    return -1;
}

int &AppBlynk::getIntCacheValue(String pinId) {
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
    if (pinId == "soilMoisture3") return soilMoistureCache3;
    if (pinId == "soilMoisture4") return soilMoistureCache4;

    return fishIntCache;
}

String &AppBlynk::getStringCacheValue(String pinId) {
    if (pinId == "otaHost") return otaHostCache;
    if (pinId == "otaBin") return otaBinCache;
    if (pinId == "otaLastUpdateTime") return otaLastUpdateTimeCache;
    if (pinId == "uptime") return uptimeCache;

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
        if (intVariables[i].pin == pin) {
            return *stringVariables[i].var;
        }
    }

    return fishStringCache;
}

void sync() { // every 60 sec
    AppBlynk::postData("otaLastUpdateTime", EspOta::getUpdateTime());
    AppBlynk::postData("uptime", Tools::getUptime());
    AppBlynk::postData("version", VERSION_MARKER + String(VERSION));
}

void syncHighFreq() { // every 2 sec
    AppBlynk::postData("temperature", Sensor::temperatureGet());
    AppBlynk::postData("humidity", Sensor::humidityGet());
    AppBlynk::postData("light", Relay::isLightOn() ? 255 : 0);
    AppBlynk::postData("ventilation", Relay::isVentilationOn() ? 255 : 0);
    AppBlynk::postData("rtcBattery", AppTime::RTCBattery() ? 255 : 0);
    AppBlynk::postData("rtcTemperature", AppTime::RTCGetTemperature());
    AppBlynk::postData("humidityWater", Sensor::humidityHasWater() ? 255 : 0);
    AppBlynk::postData("soilMoisture1", Sensor::getSoilMoisture(SOIL_SENSOR_1, SOIL_SENSOR_1_MIN, SOIL_SENSOR_1_MAX));
    AppBlynk::postData("soilMoisture2", Sensor::getSoilMoisture(SOIL_SENSOR_2, SOIL_SENSOR_2_MIN, SOIL_SENSOR_2_MAX));
    AppBlynk::postData("soilMoisture3", Sensor::getSoilMoisture(SOIL_SENSOR_3, SOIL_SENSOR_3_MIN, SOIL_SENSOR_3_MAX));
    AppBlynk::postData("soilMoisture4", Sensor::getSoilMoisture(SOIL_SENSOR_4, SOIL_SENSOR_4_MIN, SOIL_SENSOR_4_MAX));

    bool humidityHasWater = Sensor::humidityHasWater();
#if PRODUCTION
    if (!humidityHasWater && !noHumidityWaterNotificationSent) {
        Blynk.notify("Humidity has no water");
        noHumidityWaterNotificationSent = true;
    } else if (humidityHasWater && noHumidityWaterNotificationSent) {
        noHumidityWaterNotificationSent = false;
    }
#endif
}

BLYNK_WRITE(V6) { // lightDayStart
    const char* pin = "lightDayStart";
    int& variable = AppBlynk::getIntVariable(pin);
    AppBlynk::getData(variable, pin, param.asInt(), true);
}
BLYNK_WRITE(V7) { // lightDayEnd
    const char* pin = "lightDayEnd";
    int& variable = AppBlynk::getIntVariable(pin);
    AppBlynk::getData(variable, pin, param.asInt(), true);
}
BLYNK_WRITE(V8) { // ventTempMax
    const char* pin = "ventTempMax";
    int& variable = AppBlynk::getIntVariable(pin);
    AppBlynk::getData(variable, pin, param.asInt(), true);
}
BLYNK_WRITE(V14) { // ventHumMax
    const char* pin = "ventHumMax";
    int& variable = AppBlynk::getIntVariable(pin);
    AppBlynk::getData(variable, pin, param.asInt(), true);
}
BLYNK_WRITE(V20) { // otaHost
    const char* pin = "otaHost";
    String& variable = AppBlynk::getStringVariable(pin);
    AppBlynk::getData(variable, pin, param.asStr(), true);
}
BLYNK_WRITE(V21) { // otaBin
    const char* pin = "otaBin";
    String& variable = AppBlynk::getStringVariable(pin);
    AppBlynk::getData(variable, pin, param.asStr(), true);
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
    int urlCounts = *(&intVariables + 1) - intVariables;
    for (int i = 0; i < urlCounts; i++) {
        if (!intVariables[i].pin) {
            intVariables[i] = BlynkIntVariable(var, pin, store);
            break;
        }
    }
}

void AppBlynk::setVariable(String *var, const char *pin, bool store) {
    int urlCounts = *(&stringVariables + 1) - stringVariables;
    for (int i = 0; i < urlCounts; i++) {
        if (!stringVariables[i].pin) {
            stringVariables[i] = BlynkStringVariable(var, pin, store);
            break;
        }
    }
}

void AppBlynk::initiate() {
    Blynk.begin(blynkAuth, AppWiFi::getSSID(), AppWiFi::getPSWD(), blynkDomain, blynkPort);
    blynkTerminal.clear();
    sync();
    blynkSyncTimer.setInterval(blynkSyncInterval, sync);
    blynkSyncHighFreqTimer.setInterval(blynkSyncHighFreqInterval, syncHighFreq);
}

void AppBlynk::run() {
    Blynk.run();
    blynkSyncTimer.run();
    blynkSyncHighFreqTimer.run();
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

void AppBlynk::getData(String &localVariable, const char *pinId, String pinData, bool storePreferences) {
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
        Blynk.virtualWrite(blynkPin, value);
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
        Blynk.virtualWrite(blynkPin, value);
        cacheValue = value;
    }
}

void AppBlynk::terminal(String value) {
    Serial.print(value);
    blynkTerminal.println(value);
    blynkTerminal.flush();
}