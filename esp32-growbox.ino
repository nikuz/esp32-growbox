#include <EspOta.h>
#include <Ticker.h>

#include "def.h"
#include "AppWiFi.h"
#include "AppStorage.h"
#include "AppTime.h"
#include "Sensor.h"
#include "Tools.h"
#include "Screen.h"
#include "Relay.h"
#include "Blynk.h"

static AppStorage appStorage;
static Tools tools;
static AppTime appTime;
static Relay relay;

static const char* TAG = "growbox";

// OTA settings
String otaHost = "selfproduct.com";
const int otaPort = 80;  // Non https. For HTTPS 443. As of today, HTTPS doesn't work.
#if PRODUCTION
String otaBin = "/esp32-updates/growbox.bin";
#else
String otaBin = "/esp32-updates/growbox-dev.bin";
#endif
EspOta otaUpdate(otaHost, otaPort, otaBin, TAG);
int otaCheckUpdateInterval = 60 * 2;  // check OTA update every 2 minutes
Ticker otaCheckUpdateTimer;

static AppWiFi appWiFiClient;
const int wifiCheckConnectionInterval = 30;  // check Wifi connect every 30 seconds
Ticker wifiCheckConnectionTimer;

static Screen screen;
const int screenRefreshInterval = 5; // refresh screen every 5 seconds
Ticker screenRefreshTimer;

// temperature/humidity settings
static Sensor sensor;
const int sensorsReadInterval = 5;  // read sensor every 5 seconds
Ticker sensorsReadTimer;

// ventilation settings
const int ventilationProphylaxisInterval = 60;  // check ventilation every minute
int ventHumMax = 40; // ventilationHumidityMax
int ventTempMax = 35; // ventilationTemperatureMax
Ticker ventilationProphylaxisTimer;

// light settings
const int hourCheckInterval = 5;  // check current hour every 5 seconds
Ticker hourCheckTimer;
int lightDayStart = 22;  // 22:00 by default
int lightDayEnd = 15;    // 15:00 by default

// Blynk settings
static Blynk blynkClient;
const int blynkSyncHighFreqInterval = 5;  // sync blynk state every 5 seconds
Ticker blynkSyncHighFreqTimer;
const int blynkSyncInterval = 60;  // sync blynk state every 5 seconds
Ticker blynkSyncTimer;
int pingNeedResponse = 0;
int timeNeedResponse = 0;
int deviceRestartNeed = 0;
bool noHumidityWaterNotificationSended = false;

void wifiConnect() {
    appWiFiClient.connect();
}

void otaUpdateHandler() { 
    if (otaUpdate._host != otaHost || otaUpdate._bin != otaBin) {
        otaUpdate.updateEntries(otaHost, otaPort, otaBin);
    }
    otaUpdate.begin(appTime.getTimeString(appTime.getCurrentTime()));
}

void screenRefresh() {
    screen.clearBuffer();
    screen.printTemperature(sensor.temperatureGet(), sensor.humidityGet());
    screen.printDayStrip(appTime.getCurrentHour(), lightDayStart, lightDayEnd);
    screen.printAppVersion();
    screen.printTime(appTime.getCurrentTime());
    screen.printHumidityWater(sensor.humidityHasWater());
    screen.sendBuffer();
}

void sensorsRead() {
    sensor.readDHT();
    if (
        sensor.temperatureMoreThan(ventTempMax) 
        || sensor.humidityMoreThan(ventHumMax + 10)
    ) {
        relay.ventilationOn();
    } else if (
        !relay.isVentilationProphylaxisOn()
        && sensor.temperatureLessThan(ventTempMax) 
        && sensor.humidityLessThan(ventHumMax + 10)
    ) {
        relay.ventilationOff();
    }

    if (sensor.humidityLessThan(ventHumMax - 10) && sensor.humidityHasWater()) {
        relay.humidityOn();
    } else if (sensor.humidityMoreThan(ventHumMax)) {
        relay.humidityOff();
    }
}

void ventilationProphylaxis() {
    relay.ventilationProphylaxis();
}

void hourCheck() {
    if (tools.lightDayDiapasonMatch(appTime.getCurrentHour(), lightDayStart, lightDayEnd)) {
        relay.lightOn();
    } else {
        relay.lightOff();
    }
}

void blynkSyncHighFreq() { // every 5 sec
    // next function will get whole blynk project in JSON and parse it to get pins values
    // if new pins values changed it will be stored in local variables and storage
    blynkClient.getProject();

    if (pingNeedResponse == 1) {
        pingNeedResponse = 0;
        blynkClient.notification("PONG");
        blynkClient.postData("ping", 0);
    }

    if (timeNeedResponse == 1) {
        timeNeedResponse = 0;
        appTime.print();
        blynkClient.postData("time", 0);
    }

    if (deviceRestartNeed == 1) {
        deviceRestartNeed = 0;
        blynkClient.postData("restart", 0);
        delay(2000);
        ESP.restart();
    }

    bool humidityHasWater = sensor.humidityHasWater();
    #if PRODUCTION
    if (!humidityHasWater && !noHumidityWaterNotificationSended) {
        blynkClient.notification("Humidity has no water");
        noHumidityWaterNotificationSended = true;
    } else if (humidityHasWater && noHumidityWaterNotificationSended) {
        noHumidityWaterNotificationSended = false;
    }
    #endif

    blynkClient.postData("temperature", sensor.temperatureGet());
    blynkClient.postData("humidity", sensor.humidityGet());
    blynkClient.postData("light", relay.isLightOn() ? 255 : 0);
    blynkClient.postData("lightDayStart", lightDayStart);
    blynkClient.postData("lightDayEnd", lightDayEnd);
    blynkClient.postData("ventilation", relay.isVentilationOn() ? 255 : 0);
    blynkClient.postData("ventTempMax", ventTempMax);
    blynkClient.postData("ventHumMax", ventHumMax);
    blynkClient.postData("rtcBattery", appTime.RTCBattery() ? 255 : 0);
    blynkClient.postData("otaHost", otaHost);
    blynkClient.postData("otaBin", otaBin);
    blynkClient.postData("rtcTemperature", appTime.RTCGetTemperature());
    blynkClient.postData("humidityWater", humidityHasWater ? 255 : 0);
    blynkClient.postData("soilMoisture1", sensor.getSoilMoisture(SOIL_SENSOR_1, SOIL_SENSOR_1_MIN, SOIL_SENSOR_1_MAX));
    blynkClient.postData("soilMoisture2", sensor.getSoilMoisture(SOIL_SENSOR_2, SOIL_SENSOR_2_MIN, SOIL_SENSOR_2_MAX));
    blynkClient.postData("soilMoisture3", sensor.getSoilMoisture(SOIL_SENSOR_3, SOIL_SENSOR_3_MIN, SOIL_SENSOR_3_MAX));
    blynkClient.postData("soilMoisture4", sensor.getSoilMoisture(SOIL_SENSOR_4, SOIL_SENSOR_4_MIN, SOIL_SENSOR_4_MAX));
}

void blynkSync() { // every 60 sec
    blynkClient.postData("otaLastUpdateTime", otaUpdate.getUpdateTime());
    blynkClient.postData("uptime", tools.getUptime());
    blynkClient.postData("version", VERSION_MARKER + String(VERSION));
}

void setup() {
    // set relay off by default
    relay.initiate();

    // Begin Serial
    Serial.begin(115200);
    while (!Serial) {
        ;  // wait for serial port to connect. Needed for native USB
    }

    // restore preferences
    appStorage.setVariable(&lightDayStart, "lightDayStart");
    appStorage.setVariable(&lightDayEnd, "lightDayEnd");
    appStorage.setVariable(&ventTempMax, "ventTempMax");
    appStorage.setVariable(&ventHumMax, "ventHumMax");
    #if PRODUCTION
    appStorage.setVariable(&otaHost, "otaHost");
    appStorage.setVariable(&otaBin, "otaBin");
    #endif
    appStorage.restore();
    
    // register Blynk variables
    blynkClient.setVariable(&lightDayStart, "lightDayStart");
    blynkClient.setVariable(&lightDayEnd, "lightDayEnd");
    blynkClient.setVariable(&ventTempMax, "ventTempMax");
    blynkClient.setVariable(&ventHumMax, "ventHumMax");
    blynkClient.setVariable(&pingNeedResponse, "ping", false);
    blynkClient.setVariable(&timeNeedResponse, "time", false);
    blynkClient.setVariable(&deviceRestartNeed, "restart", false);
    #if PRODUCTION
    blynkClient.setVariable(&otaHost, "otaHost");
    blynkClient.setVariable(&otaBin, "otaBin");
    #endif

    // intiate modules
    screen.initiate();
    sensor.initiate();
    appWiFiClient.initiate();
    appWiFiClient.connect();
    appTime.obtainSNTP();
    appTime.RTCBegin();
    appTime.RTCUpdateByNtp();

    // attach timers
    wifiCheckConnectionTimer.attach(wifiCheckConnectionInterval, wifiConnect);
    //
    sensorsRead();
    sensorsReadTimer.attach(sensorsReadInterval, sensorsRead);
    //
    screenRefresh();
    screenRefreshTimer.attach(screenRefreshInterval, screenRefresh);
    //
    hourCheck();
    hourCheckTimer.attach(hourCheckInterval, hourCheck);
    //
    ventilationProphylaxisTimer.attach(ventilationProphylaxisInterval, ventilationProphylaxis);
    //
    otaUpdateHandler();
    otaCheckUpdateTimer.attach(otaCheckUpdateInterval, otaUpdateHandler);
    //
    blynkSyncHighFreq();
    blynkSyncHighFreqTimer.attach(blynkSyncHighFreqInterval, blynkSyncHighFreq);
    blynkSync();
    blynkSyncTimer.attach(blynkSyncInterval, blynkSync);
}

void loop() {
    ;
}
