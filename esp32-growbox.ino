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
#include "AppBlynk.h"

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

const int wifiCheckConnectionInterval = 30;  // check Wifi connect every 30 seconds
Ticker wifiCheckConnectionTimer;

const int screenRefreshInterval = 2; // refresh screen every 5 seconds
Ticker screenRefreshTimer;

// temperature/humidity settings
const int sensorsReadInterval = 2;  // read sensor every 5 seconds
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

void wifiConnect() {
    AppWiFi::connect();
}

void otaUpdateHandler() { 
    if (otaUpdate._host != otaHost || otaUpdate._bin != otaBin) {
        otaUpdate.updateEntries(otaHost, otaPort, otaBin);
    }
    otaUpdate.begin(AppTime::getTimeString(AppTime::getCurrentTime()));
}

void screenRefresh() {
    Screen::clearBuffer();
    Screen::printTemperature(Sensor::temperatureGet(), Sensor::humidityGet());
    Screen::printDayStrip(AppTime::getCurrentHour(), lightDayStart, lightDayEnd);
    Screen::printAppVersion();
    Screen::printTime(AppTime::getCurrentTime());
    Screen::printHumidityWater(Sensor::humidityHasWater());
    Screen::sendBuffer();
}

void sensorsRead() {
    Sensor::readDHT();
    if (
        Sensor::temperatureMoreThan(ventTempMax)
        || Sensor::humidityMoreThan(ventHumMax + 10)
    ) {
        Relay::ventilationOn();
    } else if (
        !Relay::isVentilationProphylaxisOn()
        && Sensor::temperatureLessThan(ventTempMax)
        && Sensor::humidityLessThan(ventHumMax + 10)
    ) {
        Relay::ventilationOff();
    }

    if (Sensor::humidityMoreThan(0) && Sensor::humidityLessThan(ventHumMax - 10) && Sensor::humidityHasWater()) {
        Relay::humidityOn();
    } else if (Sensor::humidityMoreThan(ventHumMax)) {
        Relay::humidityOff();
    }
}

void ventilationProphylaxis() {
    Relay::ventilationProphylaxis();
}

void hourCheck() {
	if (Tools::lightDayDiapasonMatch(AppTime::getCurrentHour(), lightDayStart, lightDayEnd)) {
        Relay::lightOn();
    } else {
        Relay::lightOff();
    }
}

void setup() {
    // set relay off by default
    Relay::initiate();

    // Begin Serial
    Serial.begin(115200);
    while (!Serial) {
        ;  // wait for serial port to connect. Needed for native USB
    }

    // restore preferences
    AppStorage::setVariable(&lightDayStart, "lightDayStart");
    AppStorage::setVariable(&lightDayEnd, "lightDayEnd");
    AppStorage::setVariable(&ventTempMax, "ventTempMax");
    AppStorage::setVariable(&ventHumMax, "ventHumMax");
    #if PRODUCTION
    AppStorage::setVariable(&otaHost, "otaHost");
    AppStorage::setVariable(&otaBin, "otaBin");
    #endif
    AppStorage::restore();

    // intiate modules
    Screen::initiate();
    Sensor::initiate();
    AppWiFi::initiate();
    AppWiFi::connect();
    AppTime::obtainSNTP();
    AppTime::RTCBegin();
    AppTime::RTCUpdateByNtp();

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
    // register Blynk variables
	AppBlynk::setVariable(&lightDayStart, "lightDayStart");
	AppBlynk::setVariable(&lightDayEnd, "lightDayEnd");
	AppBlynk::setVariable(&ventTempMax, "ventTempMax");
	AppBlynk::setVariable(&ventHumMax, "ventHumMax");
	#if PRODUCTION
	AppBlynk::setVariable(&otaHost, "otaHost");
	AppBlynk::setVariable(&otaBin, "otaBin");
	#endif

//    AppBlynk::initiate();
}

void loop() {
//    AppBlynk::run();
}
