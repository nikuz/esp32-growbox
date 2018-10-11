#include <EspOta.h>
#include <Ticker.h>

#include "def.h"
#include "AppWiFi.h"
#include "AppStorage.h"
#include "AppTime.h"
#include "AppDHT.h"
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
static AppDHT appDHT;
const int DHTPReadDataInterval = 5;  // read sensor every 5 seconds
Ticker DHTPReadDataTimer;

// ventilation settings
const int ventilationProphylaxisInterval = 60;  // check ventilation every minute
const int ventHumMax = 70; // ventilationHumidityMax
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
    screen.printTemperature(appDHT.temperatureGet(), appDHT.humidityGet());
    screen.printDayStrip(appTime.getCurrentHour(), lightDayStart, lightDayEnd);
    screen.printAppVersion();
    screen.printTime(appTime.getCurrentTime());
    screen.sendBuffer();
    //
    // appTime.print();
}

void DHTPRead() {
    appDHT.read();
    if (
        appDHT.temperatureMoreThan(ventTempMax) 
        || appDHT.humidityMoreThan(ventHumMax)
    ) {
        relay.ventilationOn();
    } else if (
        !relay.isVentilationProphylaxisOn()
        && appDHT.temperatureLessThan(ventTempMax) 
        && appDHT.humidityLessThan(ventHumMax)
    ) {
        relay.ventilationOff();
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
        blynkClient.pingResponse();
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

    blynkClient.postData("temperature", appDHT.temperatureGet());
    blynkClient.postData("humidity", appDHT.humidityGet());
    blynkClient.postData("light", relay.isLightOn() ? 255 : 0);
    blynkClient.postData("lightDayStart", lightDayStart);
    blynkClient.postData("lightDayEnd", lightDayEnd);
    blynkClient.postData("ventilation", relay.isVentilationOn() ? 255 : 0);
    blynkClient.postData("ventTempMax", ventTempMax);
    blynkClient.postData("rtcBattery", appTime.RTCBattery() ? 255 : 0);
    blynkClient.postData("otaHost", otaHost);
    blynkClient.postData("otaBin", otaBin);
    blynkClient.postData("rtcTemperature", appTime.RTCGetTemperature());
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
    #if PRODUCTION
    appStorage.setVariable(&otaHost, "otaHost");
    appStorage.setVariable(&otaBin, "otaBin");
    #endif
    appStorage.restore();
    
    // register Blynk variables
    blynkClient.setVariable(&lightDayStart, "lightDayStart");
    blynkClient.setVariable(&lightDayEnd, "lightDayEnd");
    blynkClient.setVariable(&ventTempMax, "ventTempMax");
    blynkClient.setVariable(&pingNeedResponse, "ping", false);
    blynkClient.setVariable(&timeNeedResponse, "time", false);
    blynkClient.setVariable(&deviceRestartNeed, "restart", false);
    #if PRODUCTION
    blynkClient.setVariable(&otaHost, "otaHost");
    blynkClient.setVariable(&otaBin, "otaBin");
    #endif

    // intiate modules
    screen.initiate();
    appDHT.initiate();
    appWiFiClient.initiate();
    appWiFiClient.connect();
    appTime.obtainSNTP();
    appTime.RTCBegin();
    appTime.RTCUpdateByNtp();

    // attach timers
    wifiCheckConnectionTimer.attach(wifiCheckConnectionInterval, wifiConnect);
    //
    DHTPRead();
    DHTPReadDataTimer.attach(DHTPReadDataInterval, DHTPRead);
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
