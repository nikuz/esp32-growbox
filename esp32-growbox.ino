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
AppTime timer;

// OTA settings
String otaHost = "selfproduct.com";
const int otaPort = 80;  // Non https. For HTTPS 443. As of today, HTTPS doesn't work.
#if PRODUCTION
String otaBin = "/esp32-updates/growbox.bin";
#else
String otaBin = "/esp32-updates/growbox-dev.bin";
#endif
#ifdef DEBUG
EspOta otaUpdate(otaHost, otaPort, otaBin, TAG, true);
#else
EspOta otaUpdate(otaHost, otaPort, otaBin, TAG);
#endif
const unsigned long otaCheckUpdateInterval = 60UL * 1000UL;  // check OTA update every minute

const unsigned long screenRefreshInterval = 2UL * 1000UL; // refresh screen every 2 seconds

// temperature/humidity settings
const unsigned long sensorsReadInterval = 2UL * 1000UL;  // read sensor every 2 seconds

// ventilation settings
const unsigned long ventilationProphylaxisInterval = 60UL * 1000UL;  // check ventilation every minute
int ventHumMax = 65;  // ventilationHumidityMax
int ventTempMax = 35; // ventilationTemperatureMax
int windTempMin = 18;
int windHumMin = 35;

// light settings
const unsigned long hourCheckInterval = 1UL * 1000UL;  // check current hour every second
int lightDayStart = 0;  // 00:00 by default
int lightDayEnd = 17;   // 17:00 by default
int lightMaxInt = 20;   // in percents
const unsigned long setIntensityInterval = 60UL * 1000UL; // set light intensity once in minute

// watering
const unsigned long wateringInterval = 5UL * 1000UL; // check every 5 seconds
const unsigned long wateringProgressCheckInterval = 1UL * 1000UL;  // check every second
int autoWatering = 0; // auto watering disabled by default
int wSoilMstrMin = 30;
String s1LstWtrng = "";
String s2LstWtrng = "";
String s3LstWtrng = "";
String s4LstWtrng = "";
String hLstWtrng = "";
// next gives ability to enable watering for special pot manually
int s1WtrngAuto = 0;
int s2WtrngAuto = 0;
int s3WtrngAuto = 0;
int s4WtrngAuto = 0;
int hWtrngAuto = 0;

const unsigned long blynkSyncInterval = 60UL * 1000UL;  // sync blynk low freq state every 60 seconds
const unsigned long blynkSyncHighFreqInterval = 3UL * 1000UL;  // sync blynk high freq state every 3 seconds
const unsigned long blynkSyncHighFreqInterval2 = 4UL * 1000UL; // 4 sec
const unsigned long blynkSyncHighFreqInterval3 = 5UL * 1000UL; // 5 sec
const unsigned long blynkCheckConnectInterval = 30UL * 1000UL;  // check blynk connection every 30 seconds

const unsigned long uptimePrintInterval = 1UL * 1000UL;

void otaUpdateHandler() {
    if (Tools::millisOverflowIsClose()) {
        return;
    }
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
    } else if (!Sensor::humidityHasWater() || Sensor::humidityMoreThan(ventHumMax)) {
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
    AppStorage::setVariable(&s1LstWtrng, "s1LstWtrng");
    AppStorage::setVariable(&s2LstWtrng, "s2LstWtrng");
    AppStorage::setVariable(&s3LstWtrng, "s3LstWtrng");
    AppStorage::setVariable(&s4LstWtrng, "s4LstWtrng");
    AppStorage::setVariable(&hLstWtrng, "hLstWtrng");
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
    Watering::setVariable(&s1LstWtrng, "s1LstWtrng");
    Watering::setVariable(&s2LstWtrng, "s2LstWtrng");
    Watering::setVariable(&s3LstWtrng, "s3LstWtrng");
    Watering::setVariable(&s4LstWtrng, "s4LstWtrng");
    Watering::setVariable(&hLstWtrng, "hLstWtrng");
    Watering::setVariable(&s1WtrngAuto, "s1WtrngAuto");
    Watering::setVariable(&s2WtrngAuto, "s2WtrngAuto");
    Watering::setVariable(&s3WtrngAuto, "s3WtrngAuto");
    Watering::setVariable(&s4WtrngAuto, "s4WtrngAuto");
    Watering::setVariable(&hWtrngAuto, "hWtrngAuto");

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
    AppBlynk::setVariable(&s1WtrngAuto, "s1WtrngAuto");
    AppBlynk::setVariable(&s2WtrngAuto, "s2WtrngAuto");
    AppBlynk::setVariable(&s3WtrngAuto, "s3WtrngAuto");
    AppBlynk::setVariable(&s4WtrngAuto, "s4WtrngAuto");
    AppBlynk::setVariable(&hWtrngAuto, "hWtrngAuto");

    // start Blynk connection
    AppBlynk::initiate();

    Light::setVariable(&lightMaxInt, "lightMaxInt");
    Light::setIntensity();

    timer.setInterval(sensorsReadInterval, sensorsRead);
    timer.setInterval(hourCheckInterval, hourCheck);
    timer.setInterval(ventilationProphylaxisInterval, Relay::ventilationProphylaxis);
    timer.setInterval(wateringInterval, Watering::check);
    timer.setInterval(wateringProgressCheckInterval, Watering::checkProgress);
    timer.setInterval(screenRefreshInterval, Screen::refresh);
    timer.setInterval(setIntensityInterval, Light::setIntensity);
    timer.setInterval(otaCheckUpdateInterval, otaUpdateHandler);
    timer.setInterval(blynkCheckConnectInterval, AppBlynk::checkConnect);
    timer.setInterval(blynkSyncInterval, AppBlynk::sync);
    timer.setInterval(blynkSyncHighFreqInterval, AppBlynk::syncHighFreq1);
    timer.setInterval(blynkSyncHighFreqInterval2, AppBlynk::syncHighFreq2);
    timer.setInterval(blynkSyncHighFreqInterval3, AppBlynk::syncHighFreq3);
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

    timer.run();
}
