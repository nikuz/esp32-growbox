#include <EspOta.h>

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

// OTA settings
String otaHost = "selfproduct.com";
const int otaPort = 80;  // Non https. For HTTPS 443. As of today, HTTPS doesn't work.
#if PRODUCTION
String otaBin = "/esp32-updates/growbox.bin";
#else
String otaBin = "/esp32-updates/growbox-dev.bin";
#endif
EspOta otaUpdate(otaHost, otaPort, otaBin, TAG);
int otaCheckUpdateInterval = 60;  // check OTA update every minute
unsigned long otaCheckUpdateLastTime = 0;

const int screenRefreshInterval = 2; // refresh screen every 2 seconds
unsigned long screenRefreshLastTime = 0;

// temperature/humidity settings
const int sensorsReadInterval = 2;  // read sensor every 2 seconds
unsigned long sensorsReadLastTime = 0;

// ventilation settings
const int ventilationProphylaxisInterval = 60;  // check ventilation every minute
int ventHumMax = 65; // ventilationHumidityMax
int ventTempMax = 35; // ventilationTemperatureMax
int windTempMin = 18;
int windHumMin = 35;
int wSoilMstrMin = 30;
unsigned long ventilationProphylaxisLastTime = 0;

// light settings
const int hourCheckInterval = 1;  // check current hour every second
unsigned long hourCheckLastTime = 0;
int lightDayStart = 0;  // 00:00 by default
int lightDayEnd = 17;   // 17:00 by default
int lightMaxInt = 50;   // in percents

void otaUpdateHandler() {
	if (otaUpdate._host != otaHost || otaUpdate._bin != otaBin) {
        otaUpdate.updateEntries(otaHost, otaPort, otaBin);
    }
    otaUpdate.begin(AppTime::getTimeString(AppTime::getCurrentTime()));
}

void screenRefresh() {
    const int currentHour = AppTime::getCurrentHour();

    Screen::clearBuffer();
    Screen::printTemperature(Sensor::temperatureGet(), Sensor::humidityGet());
    Screen::printHumidityWater(Sensor::humidityHasWater());
    Screen::printAppVersion();

    if (currentHour != -1) {
        Screen::printDayStrip(currentHour, lightDayStart, lightDayEnd);
        Screen::printTime(AppTime::getCurrentTime());
    }

    Screen::sendBuffer();
}

void sensorsRead() {
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

    if (Sensor::doorIsOpen()) {
        Relay::humidityOff();
        Relay::windOn();
        Relay::ventilationOff();
    }
}

void hourCheck() {
    const int currentHour = AppTime::getCurrentHour();
    if (currentHour == -1) {
        return;
    }
    if (Tools::lightDayDiapasonMatch(currentHour, lightDayStart, lightDayEnd)) {
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
    Relay::ventilationOff();
    Relay::humidityOff();
    Relay::lightOff();
    Relay::windOff();
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
    AppStorage::restore();

    // setup wifi ip address etc.
    AppWiFi::connect();

    //get internet time
    AppTime::obtainSNTP();

    // update RTC time on Mega by internet time
    struct tm ntpTime = {0};
    if (AppTime::localTime(&ntpTime)) {
        const char *timeParam = AppTime::getTimeString(ntpTime);
        SerialFrame timeFrame = SerialFrame("time", timeParam);
        AppSerial::sendFrame(&timeFrame);
    }

    // register Blynk variables
    AppBlynk::setVariable(&lightDayStart, "lightDayStart");
    AppBlynk::setVariable(&lightDayEnd, "lightDayEnd");
    AppBlynk::setVariable(&ventTempMax, "ventTempMax");
    AppBlynk::setVariable(&ventHumMax, "ventHumMax");
    AppBlynk::setVariable(&otaHost, "otaHost");
    AppBlynk::setVariable(&otaBin, "otaBin");
    AppBlynk::setVariable(&wSoilMstrMin, "wSoilMstrMin");
    AppBlynk::setVariable(&lightMaxInt, "lightMaxInt");

    // start Blynk connection
    AppBlynk::initiate();
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

    if (Tools::timerCheck(sensorsReadInterval, sensorsReadLastTime)) {
        sensorsRead();
        sensorsReadLastTime = millis();
    }
    if (Tools::timerCheck(hourCheckInterval, hourCheckLastTime)) {
        hourCheck();
        hourCheckLastTime = millis();
    }
    if (Tools::timerCheck(ventilationProphylaxisInterval, ventilationProphylaxisLastTime)) {
        Relay::ventilationProphylaxis();
        ventilationProphylaxisLastTime = millis();
    }
    if (Tools::timerCheck(screenRefreshInterval, screenRefreshLastTime)) {
        screenRefresh();
        screenRefreshLastTime = millis();
    }
    if (Tools::timerCheck(otaCheckUpdateInterval, otaCheckUpdateLastTime)) {
        otaUpdateHandler();
        otaCheckUpdateLastTime = millis();
    }

    Watering::check(wSoilMstrMin);
    Light::setIntensity(lightDayStart, lightDayEnd, lightMaxInt);

    AppBlynk::run();
}
