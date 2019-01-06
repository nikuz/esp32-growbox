#include <EspOta.h>
#include <SimpleTimer.h>

#include "def.h"
#include "AppWiFi.h"
#include "AppStorage.h"
#include "AppTime.h"
#include "Sensor.h"
#include "Tools.h"
#include "Screen.h"
#include "Relay.h"
#include "AppSerial.h"
#include "AppBlynk.h"
#include "Watering.h"
#include "Light.h"

static const char *TAG = "growbox";
static SimpleTimer *timer = AppTime::getTimer();

// OTA settings
String otaHost = "selfproduct.com";
const int otaPort = 80;  // Non https. For HTTPS 443. As of today, HTTPS doesn't work.
#if PRODUCTION
String otaBin = "/esp32-updates/growbox.bin";
#else
String otaBin = "/esp32-updates/growbox-dev.bin";
#endif
EspOta otaUpdate(otaHost, otaPort, otaBin, TAG);
int otaCheckUpdateInterval = 60L * 1000L;  // check OTA update every minute

const int screenRefreshInterval = 2L * 1000L; // refresh screen every 2 seconds

// temperature/humidity settings
const int sensorsReadInterval = 2L * 1000L;  // read sensor every 2 seconds

// ventilation settings
const int ventilationProphylaxisInterval = 60L * 1000L;  // check ventilation every minute
int ventHumMax = 65;  // ventilationHumidityMax
int ventTempMax = 35; // ventilationTemperatureMax
int windTempMin = 18;
int windHumMin = 35;

// light settings
const int hourCheckInterval = 1L * 1000L;  // check current hour every second
int lightDayStart = 0;  // 00:00 by default
int lightDayEnd = 17;   // 17:00 by default
int lightMaxInt = 50;   // in percents
unsigned long setIntensityInterval = 60L * 1000L; // set light intensity once in minute

// watering
int wateringInterval = 5L * 1000L; // check every 5 seconds
int autoWatering = 0; // auto watering disabled by default
int wSoilMstrMin = 30;

const int blynkSyncInterval = 60L * 1000L;  // sync blynk low freq state every 60 seconds
const int blynkSyncHighFreqInterval = 2L * 1000L;  // sync blynk high freq state every 2 seconds

void otaUpdateHandler() {
	if (otaUpdate._host != otaHost || otaUpdate._bin != otaBin) {
        otaUpdate.updateEntries(otaHost, otaPort, otaBin);
    }
    otaUpdate.begin(AppTime::getTimeString(AppTime::getCurrentTime()));
}

void sensorsRead() {
    if (Sensor::doorIsOpen()) {
        Relay::humidityOff();
        Relay::windOff();
        Relay::ventilationOff();
        return;
    }

    // ventilation
    if (Sensor::temperatureMoreThan(ventTempMax) || Sensor::humidityMoreThan(ventHumMax + 10)) {
        Relay::ventilationOn();
    } else if (
        !Relay::isVentilationProphylaxisOn()
        && Sensor::temperatureLessThan(ventTempMax)
        && Sensor::humidityLessThan(ventHumMax + 10)
    ) {
        Relay::ventilationOff();
    }

    // humidity
    if (Sensor::humidityMoreThan(0) && Sensor::humidityLessThan(ventHumMax - 10) && Sensor::humidityHasWater()) {
        Relay::humidityOn();
    } else if (Sensor::humidityMoreThan(ventHumMax)) {
        Relay::humidityOff();
    }

    // wind
    if (Sensor::temperatureLessThan(windTempMin) || Sensor::humidityLessThan(windHumMin)) {
        Relay::windOff();
    } else {
        Relay::windOn();
    }
}

void hourCheck() {
    const int currentHour = AppTime::getCurrentHour();
    if (currentHour == -1) {
        return;
    }
    if (AppTime::lightDayDiapasonMatch(currentHour)) {
        Relay::lightOn();
    } else {
        Relay::lightOff();
    }
}

void setup() {
    // initiate screen first to show loading state
    Screen::initiate();

    // Begin debug Serial
    Serial.begin(115200);
    while (!Serial) {
        ;
    }

    // Begin Mega communication Serial
    Serial2.begin(115200);
    while (!Serial2) {
        ;
    }

    // initially off all the loads
//    Relay::ventilationOff();
//    Relay::humidityOff();
//    Relay::lightOff();
//    Relay::windOff();
    Relay::wateringMixingOff();
    Relay::wateringCloseValve("s1");
    Relay::wateringCloseValve("s2");
    Relay::wateringCloseValve("s3");
    Relay::wateringCloseValve("s4");
    Relay::wateringCloseValve("sHumidity");
    Relay::wateringOff();

    // restore preferences
    AppStorage::setVariable(&lightDayStart, "lightDayStart");
    AppStorage::setVariable(&lightDayEnd, "lightDayEnd");
    AppStorage::setVariable(&ventTempMax, "ventTempMax");
    AppStorage::setVariable(&ventHumMax, "ventHumMax");
    AppStorage::setVariable(&otaHost, "otaHost");
    AppStorage::setVariable(&otaBin, "otaBin");
    AppStorage::setVariable(&wSoilMstrMin, "wSoilMstrMin");
    AppStorage::setVariable(&lightMaxInt, "lightMaxInt");
    AppStorage::setVariable(&autoWatering, "autoWatering");
    AppStorage::restore();

    // setup wifi ip address etc.
    AppWiFi::connect();

    //get internet time
    AppTime::setVariable(&lightDayStart, "lightDayStart");
    AppTime::setVariable(&lightDayEnd, "lightDayEnd");
    AppTime::obtainSNTP();

    // update RTC time on Mega by internet time
    struct tm ntpTime = {0};
    if (AppTime::localTime(&ntpTime)) {
        const char *timeParam = AppTime::getTimeString(ntpTime);
        SerialFrame timeFrame = SerialFrame("time", timeParam);
        AppSerial::sendFrame(&timeFrame);
    }

    Watering::setVariable(&autoWatering, "autoWatering");
    Watering::setVariable(&wSoilMstrMin, "wSoilMstrMin");

    // register Blynk variables
    AppBlynk::setVariable(&lightDayStart, "lightDayStart");
    AppBlynk::setVariable(&lightDayEnd, "lightDayEnd");
    AppBlynk::setVariable(&ventTempMax, "ventTempMax");
    AppBlynk::setVariable(&ventHumMax, "ventHumMax");
    AppBlynk::setVariable(&otaHost, "otaHost");
    AppBlynk::setVariable(&otaBin, "otaBin");
    AppBlynk::setVariable(&wSoilMstrMin, "wSoilMstrMin");
    AppBlynk::setVariable(&lightMaxInt, "lightMaxInt");
    AppBlynk::setVariable(&autoWatering, "autoWatering");

    // start Blynk connection
    AppBlynk::initiate();

    timer->setInterval(sensorsReadInterval, sensorsRead);
    timer->setInterval(hourCheckInterval, hourCheck);
    timer->setInterval(ventilationProphylaxisInterval, Relay::ventilationProphylaxis);
    timer->setInterval(wateringInterval, Watering::check);
    timer->setInterval(screenRefreshInterval, Screen::refresh);
    timer->setInterval(setIntensityInterval, Light::setIntensity);
    timer->setInterval(otaCheckUpdateInterval, otaUpdateHandler);
    timer->setInterval(blynkSyncInterval, AppBlynk::sync);
    timer->setInterval(blynkSyncHighFreqInterval, AppBlynk::syncHighFreq);
}

void loop() {
    // to have ability to write serial commands manually
    while (Serial.available() > 0) {
        Serial2.write(char(Serial.read()));
    }
    // read serial data from Mega
    SerialFrame serialFrame = AppSerial::getFrame();
    if (strcmp(serialFrame.command, "") != 0) {
        AppTime::parseSerialCommand(serialFrame.command, serialFrame.param);
        Sensor::parseSerialCommand(serialFrame.command, serialFrame.param);
        Relay::parseSerialCommand(serialFrame.command, serialFrame.param);
        Light::parseSerialCommand(serialFrame.command, serialFrame.param);
    }

    AppBlynk::run();

    timer->run();
}
