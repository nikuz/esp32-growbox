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
static BlynkSyncVariable syncVariables[] = {
    {"otaHost",           false},
    {"otaBin",            false},
    {"otaLastUpdateTime", false},
    {"uptime",            false},
    {"version",           false},
    {"temperature",       false},
    {"humidity",          false},
    {"light",             false},
    {"ventilation",       false},
    {"wind",              false},
    {"rtcBattery",        false},
    {"rtcTemperature",    false},
    {"humidityWater",     false},
    {"soilMoisture1",     false},
    {"soilMoisture2",     false},
    {"soilMoisture3",     false},
    {"soilMoisture4",     false},
    {"watering",          false},
    {"water",             false},
    {"waterLeakage",      false},
    {"lightIntensity",    false},
    {"door",              false},
    {"s1LstWtrng",        false},
    {"s2LstWtrng",        false},
    {"s3LstWtrng",        false},
    {"s4LstWtrng",        false},
    {"hLstWtrng",         false},
};
const int syncValuesPerSecond = 5;

AppBlynk::AppBlynk() {};

AppBlynk::~AppBlynk() {};

// private

int AppBlynk::getPinById(const char *pinId) {
    if (strcmp(pinId, "temperature") == 0) return pinTemperature;
    if (strcmp(pinId, "humidity") == 0) return pinHumidity;
    if (strcmp(pinId, "light") == 0) return pinLight;
    if (strcmp(pinId, "lightIntensity") == 0) return pinLightIntensity;
    if (strcmp(pinId, "lightMaxInt") == 0) return pinLightMaxInt;
    if (strcmp(pinId, "lightDayStart") == 0) return pinLightDayStart;
    if (strcmp(pinId, "lightDayEnd") == 0) return pinLightDayEnd;
    if (strcmp(pinId, "ventilation") == 0) return pinVentilation;
    if (strcmp(pinId, "wind") == 0) return pinWind;
    if (strcmp(pinId, "ventTempMax") == 0) return pinVentilationTemperatureMax;
    if (strcmp(pinId, "ventHumMax") == 0) return pinVentilationHumidityMax;
    if (strcmp(pinId, "version") == 0) return pinVersion;
    if (strcmp(pinId, "rtcBattery") == 0) return pinRtcBattery;
    if (strcmp(pinId, "otaHost") == 0) return pinOtaHost;
    if (strcmp(pinId, "otaBin") == 0) return pinOtaBin;
    if (strcmp(pinId, "otaLastUpdateTime") == 0) return pinOtaLastUpdateTime;
    if (strcmp(pinId, "uptime") == 0) return pinUptime;
    if (strcmp(pinId, "rtcTemperature") == 0) return pinRtcTemperature;
    if (strcmp(pinId, "humidityWater") == 0) return pinHumidityWater;
    if (strcmp(pinId, "soilMoisture1") == 0) return pinSoilMoisture1;
    if (strcmp(pinId, "soilMoisture2") == 0) return pinSoilMoisture2;
    if (strcmp(pinId, "soilMoisture3") == 0) return pinSoilMoisture3;
    if (strcmp(pinId, "soilMoisture4") == 0) return pinSoilMoisture4;
    if (strcmp(pinId, "wSoilMstrMin") == 0) return pinWSoilMstrMin;
    if (strcmp(pinId, "water") == 0) return pinWater;
    if (strcmp(pinId, "autoWatering") == 0) return pinAutoWatering;
    if (strcmp(pinId, "watering") == 0) return pinWatering;
    if (strcmp(pinId, "waterLeakage") == 0) return pinWaterLeakage;
    if (strcmp(pinId, "door") == 0) return pinDoor;
    if (strcmp(pinId, "s1LstWtrng") == 0) return pinS1LstWtrng;
    if (strcmp(pinId, "s2LstWtrng") == 0) return pinS2LstWtrng;
    if (strcmp(pinId, "s3LstWtrng") == 0) return pinS3LstWtrng;
    if (strcmp(pinId, "s4LstWtrng") == 0) return pinS4LstWtrng;
    if (strcmp(pinId, "hLstWtrng") == 0) return pinHLstWtrng;
    if (strcmp(pinId, "s1WtrngAuto") == 0) return pinS1WtrngAuto;
    if (strcmp(pinId, "s2WtrngAuto") == 0) return pinS2WtrngAuto;
    if (strcmp(pinId, "s3WtrngAuto") == 0) return pinS3WtrngAuto;
    if (strcmp(pinId, "s4WtrngAuto") == 0) return pinS4WtrngAuto;
    if (strcmp(pinId, "hWtrngAuto") == 0) return pinHWtrngAuto;

    return -1;
}

int &AppBlynk::getIntCacheValue(const char *pinId) {
    if (strcmp(pinId, "temperature") == 0) return temperatureCache;
    if (strcmp(pinId, "humidity") == 0) return humidityCache;
    if (strcmp(pinId, "light") == 0) return lightCache;
    if (strcmp(pinId, "lightIntensity") == 0) return lightIntensityCache;
    if (strcmp(pinId, "lightDayStart") == 0) return lightDayStartCache;
    if (strcmp(pinId, "lightDayEnd") == 0) return lightDayEndCache;
    if (strcmp(pinId, "ventilation") == 0) return ventilationCache;
    if (strcmp(pinId, "wind") == 0) return windCache;
    if (strcmp(pinId, "ventTempMax") == 0) return ventilationTemperatureMaxCache;
    if (strcmp(pinId, "ventHumMax") == 0) return ventilationHumidityMaxCache;
    if (strcmp(pinId, "version") == 0) return versionCache;
    if (strcmp(pinId, "rtcBattery") == 0) return rtcBatteryCache;
    if (strcmp(pinId, "rtcTemperature") == 0) return rtcTemperatureCache;
    if (strcmp(pinId, "humidityWater") == 0) return humidityWaterCache;
    if (strcmp(pinId, "soilMoisture1") == 0) return soilMoistureCache1;
    if (strcmp(pinId, "soilMoisture2") == 0) return soilMoistureCache2;
    if (strcmp(pinId, "soilMoisture3") == 0) return soilMoistureCache3;
    if (strcmp(pinId, "soilMoisture4") == 0) return soilMoistureCache4;
    if (strcmp(pinId, "wSoilMstrMin") == 0) return wSoilMstrMinCache;
    if (strcmp(pinId, "autoWatering") == 0) return autoWateringCache;
    if (strcmp(pinId, "watering") == 0) return wateringCache;
    if (strcmp(pinId, "water") == 0) return waterCache;
    if (strcmp(pinId, "waterLeakage") == 0) return pinWaterLeakageCache;
    if (strcmp(pinId, "door") == 0) return doorCache;

    return fishIntCache;
}

String &AppBlynk::getStringCacheValue(const char *pinId) {
    if (strcmp(pinId, "otaHost") == 0) return otaHostCache;
    if (strcmp(pinId, "otaBin") == 0) return otaBinCache;
    if (strcmp(pinId, "otaLastUpdateTime") == 0) return otaLastUpdateTimeCache;
    if (strcmp(pinId, "uptime") == 0) return uptimeCache;
    if (strcmp(pinId, "s1LstWtrng") == 0) return s1LstWtrngCache;
    if (strcmp(pinId, "s2LstWtrng") == 0) return s2LstWtrngCache;
    if (strcmp(pinId, "s3LstWtrng") == 0) return s3LstWtrngCache;
    if (strcmp(pinId, "s4LstWtrng") == 0) return s4LstWtrngCache;
    if (strcmp(pinId, "hLstWtrng") == 0) return hLstWtrngCache;

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

void AppBlynk::sync() { // every second
    DEBUG_PRINTLN("Check connect:");
    DEBUG_PRINT("Wifi connected: ");
    DEBUG_PRINTLN(AppWiFi::isConnected());
    DEBUG_PRINT("Blynk connected: ");
    DEBUG_PRINTLN(Blynk.connected());
    DEBUG_PRINT("Millis: ");
    DEBUG_PRINTLN(millis());
    DEBUG_PRINT("Overflow is close: ");
    DEBUG_PRINTLN(Tools::millisOverflowIsClose());
    if (!AppWiFi::isConnected() || !Blynk.connected() || Tools::millisOverflowIsClose()) {
        return;
    }

    int syncCounter = 0;
    const int varsLen = *(&syncVariables + 1) - syncVariables;
    DEBUG_PRINT("Vars to sync: ");
    DEBUG_PRINTLN(varsLen);
    for (int i = 0; i < varsLen; i++) {
        if (syncCounter < syncValuesPerSecond) {
            if (syncVariables[i].synced) {
                continue;
            }

            const char *pin = syncVariables[i].pin;
            DEBUG_PRINT("Sync pin: ");
            DEBUG_PRINT(pin);
            DEBUG_PRINT(": ");
            if (strcmp(pin, "otaHost") == 0) {
                String &otaHostVariable = AppBlynk::getStringVariable(pin);
                AppBlynk::postData(pin, otaHostVariable);
            };
            if (strcmp(pin, "otaBin") == 0) {
                String &otaBinVariable = AppBlynk::getStringVariable(pin);
                AppBlynk::postData(pin, otaBinVariable);
            };
            if (strcmp(pin, "otaLastUpdateTime") == 0) {
                AppBlynk::postData(pin, EspOta::getUpdateTime());
            };
            if (strcmp(pin, "uptime") == 0) {
                AppBlynk::postData(pin, String(Tools::getUptime()));
            };
            if (strcmp(pin, "version") == 0) {
                AppBlynk::postData(pin, VERSION);
            };
            if (strcmp(pin, "temperature") == 0) {
                AppBlynk::postData(pin, Sensor::temperatureGet());
            };
            if (strcmp(pin, "humidity") == 0) {
                AppBlynk::postData(pin, Sensor::humidityGet());
            };
            if (strcmp(pin, "light") == 0) {
                AppBlynk::postData(pin, Relay::isLightOn() ? 255 : 0);
            };
            if (strcmp(pin, "ventilation") == 0) {
                AppBlynk::postData(pin, Relay::isVentilationOn() ? 255 : 0);
            };
            if (strcmp(pin, "wind") == 0) {
                AppBlynk::postData(pin, Relay::isWindOn() ? 255 : 0);
            };
            if (strcmp(pin, "rtcBattery") == 0) {
                AppBlynk::postData(pin, AppTime::RTCBattery() ? 255 : 0);
            };
            if (strcmp(pin, "rtcTemperature") == 0) {
                AppBlynk::postData(pin, AppTime::RTCGetTemperature());
            };
            if (strcmp(pin, "humidityWater") == 0) {
                AppBlynk::postData(pin, Sensor::humidityHasWater() ? 255 : 0);
            };
            if (strcmp(pin, "soilMoisture1") == 0) {
                AppBlynk::postData(pin, Sensor::getSoilMoisture(SOIL_SENSOR_1));
            };
            if (strcmp(pin, "soilMoisture2") == 0) {
                AppBlynk::postData(pin, Sensor::getSoilMoisture(SOIL_SENSOR_2));
            };
            if (strcmp(pin, "soilMoisture3") == 0) {
                AppBlynk::postData(pin, Sensor::getSoilMoisture(SOIL_SENSOR_3));
            };
            if (strcmp(pin, "soilMoisture4") == 0) {
                AppBlynk::postData(pin, Sensor::getSoilMoisture(SOIL_SENSOR_4));
            };
            if (strcmp(pin, "watering") == 0) {
                AppBlynk::postData(pin, Relay::isWateringOn() ? 255 : 0);
            };
            if (strcmp(pin, "water") == 0) {
                AppBlynk::postData(pin, Sensor::wateringHasWater() ? 255 : 0);
            };
            if (strcmp(pin, "waterLeakage") == 0) {
                AppBlynk::postData(pin, Sensor::waterLeakageDetected() ? 255 : 0);
            };
            if (strcmp(pin, "lightIntensity") == 0) {
                AppBlynk::postData(pin, Light::intensity());
            };
            if (strcmp(pin, "door") == 0) {
                AppBlynk::postData(pin, Sensor::doorIsOpen() ? 0 : 255);
            };
            if (strcmp(pin, "s1LstWtrng") == 0) {
                AppBlynk::postData(pin, Watering::getStringVariable(pin));
            };
            if (strcmp(pin, "s2LstWtrng") == 0) {
                AppBlynk::postData(pin, Watering::getStringVariable(pin));
            };
            if (strcmp(pin, "s3LstWtrng") == 0) {
                AppBlynk::postData(pin, Watering::getStringVariable(pin));
            };
            if (strcmp(pin, "s4LstWtrng") == 0) {
                AppBlynk::postData(pin, Watering::getStringVariable(pin));
            };
            if (strcmp(pin, "hLstWtrng") == 0) {
                AppBlynk::postData(pin, Watering::getStringVariable(pin));
            };
            syncVariables[i].synced = true;
            syncCounter++;
        }
    }

    if (syncCounter < syncValuesPerSecond) {
        for (int i = 0; i < varsLen; i++) {
            syncVariables[i].synced = false;
        }
    }

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

void AppBlynk::setVariable(int *var, const char *pin) {
    int varsCount = *(&intVariables + 1) - intVariables;
    for (int i = 0; i < varsCount; i++) {
        if (!intVariables[i].pin) {
            intVariables[i] = BlynkIntVariable(var, pin);
            break;
        }
    }
}

void AppBlynk::setVariable(String *var, const char *pin) {
    int varsCount = *(&stringVariables + 1) - stringVariables;
    for (int i = 0; i < varsCount; i++) {
        if (!stringVariables[i].pin) {
            stringVariables[i] = BlynkStringVariable(var, pin);
            break;
        }
    }
}

void AppBlynk::checkConnect() {
    DEBUG_PRINTLN("Check connect:");
    DEBUG_PRINT("Wifi connected: ");
    DEBUG_PRINTLN(AppWiFi::isConnected());
    DEBUG_PRINT("Blynk connected: ");
    DEBUG_PRINTLN(Blynk.connected());
    DEBUG_PRINT("Millis: ");
    DEBUG_PRINTLN(millis());
    DEBUG_PRINT("Overflow is close: ");
    DEBUG_PRINTLN(Tools::millisOverflowIsClose());
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

void AppBlynk::postData(const char *pinId, int value) {
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

void AppBlynk::postData(const char *pinId, String value) {
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

void AppBlynk::postDataNoCache(const char *pinId, int value) {
    int blynkPin = AppBlynk::getPinById(pinId);
    if (blynkPin == -1) {
        return;
    }
    if (Blynk.connected()) {
        Blynk.virtualWrite(blynkPin, value);
    }
}

void AppBlynk::postDataNoCache(const char *pinId, String value) {
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
