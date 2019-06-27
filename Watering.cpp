#include <Arduino.h>

#include "def.h"
#include "Watering.h"
#include "AppTime.h"
#include "AppStorage.h"
#include "AppBlynk.h"
#include "Sensor.h"
#include "Relay.h"
#include "Tools.h"

static WateringIntVariable intVariables[10];
static WateringStringVariable stringVariables[10];
static WateringTargetVariable targetVariables[] = {
    {"s1",        "s1LstWtrng", "s1WtrngAuto"},
    {"s2",        "s2LstWtrng", "s2WtrngAuto"},
    {"s3",        "s3LstWtrng", "s3WtrngAuto"},
    {"s4",        "s4LstWtrng", "s4WtrngAuto"},
    {"sHumidity", "hLstWtrng",  "hWtrngAuto"}
};
static int blankIntVariable = -1;
static String blankStringVariable = "";

bool wateringStarted = false;
char wateringStartedFor[10];

bool mainWateringStarted = false;
struct tm mainWateringStartedAt = {0};
const int mainWateringTime = 30;                // 30 sec
const int mainWateringTimeHumidity = 60 * 5;    // 5 min
const int waitBetweenWatering = 60 * 5;         // 5 min
bool mainWateringPassed = false;

bool moistureRecheckPassed = false;
int moistureDifferenceThreshold = 10;

bool waitingBeforeMoistureRecheckStarted = false;
struct tm waitingBeforeMoistureRecheckStartedAt = {0};
const int waitingBeforeMoistureRecheckTime = 10; // 10 sec
bool waitingBeforeMoistureRecheckPassed = false;

bool testWateringStarted = false;
struct tm testWateringStartedAt = {0};
const int testWateringTime = 30; // 30 sec
bool testWateringPassed = false;

bool valveIsOpen = false;

bool mixingStarted = false;
struct tm mixingEnabledAt = {0};
const int mixingTime = 30; // 30 sec
bool mixingPassed = false;

int moistureStarted = 0;

Watering::Watering() {}

Watering::~Watering() {}

int &Watering::getIntVariable(const char *key) {
    const int varsLen = *(&intVariables + 1) - intVariables;
    for (int i = 0; i < varsLen; i++) {
        if (intVariables[i].key == key) {
            return *intVariables[i].var;
        }
    }

    return blankIntVariable;
}

String &Watering::getStringVariable(const char *key) {
    const int varsLen = *(&stringVariables + 1) - stringVariables;
    for (int i = 0; i < varsLen; i++) {
        if (stringVariables[i].key == key) {
            return *stringVariables[i].var;
        }
    }

    return blankStringVariable;
}

bool isWateringEnabled() {
    bool specialWateringEnabled = false;
    const int varsLen = *(&targetVariables + 1) - targetVariables;
    for (int i = 0; i < varsLen; i++) {
        if (Watering::getIntVariable(targetVariables[i].autoWateringVar)) {
            specialWateringEnabled = true;
        }
    }
    return Watering::getIntVariable("autoWatering") == 1 || specialWateringEnabled;
}

int getSoilMoisture(const char *sensorId) {
    if (strcmp(sensorId, "s1") == 0) {
        return Sensor::getSoilMoisture(SOIL_SENSOR_1);
    }
    if (strcmp(sensorId, "s2") == 0) {
        return Sensor::getSoilMoisture(SOIL_SENSOR_2);
    }
    if (strcmp(sensorId, "s3") == 0) {
        return Sensor::getSoilMoisture(SOIL_SENSOR_3);
    }
    if (strcmp(sensorId, "s4") == 0) {
        return Sensor::getSoilMoisture(SOIL_SENSOR_4);
    }

    return 0;
}

double getLastTimeWateringForPot(char *lastWateringVar) {
    String &lastWateringTime = Watering::getStringVariable(lastWateringVar);
    double lastWateringSec = AppTime::compareDates(lastWateringTime, AppTime::getCurrentTime());
    return lastWateringSec > 0 ? lastWateringSec : 0;
}

void printLastWateringTime(char *name, double prevWateringSec) {
    Serial.print("Prev watering for ");
    Serial.print(name);
    Serial.print(" was only ");
    Serial.print(prevWateringSec);
    Serial.println(" sec ago.");
    Serial.print("Must be at least ");
    Serial.print(waitBetweenWatering);
    Serial.println(" sec.");
}

bool noWaterOrLeakageDetected() {
    return !Sensor::wateringHasWater() || Sensor::waterLeakageDetected();
}

// stopping (8 stage)

void stopping() {
    if (!wateringStarted) {
        return;
    }

    if (mainWateringPassed) {
        Watering::stop();
    }
}

// main watering (7 stage)

void mainWatering() {
    if (!valveIsOpen || mainWateringPassed) {
        return;
    }
    const bool wateringForHumidity = strcmp(wateringStartedFor, "sHumidity") == 0;
    if (!wateringForHumidity && !moistureRecheckPassed) {
        return;
    }
    const int wateringTime = wateringForHumidity ? mainWateringTimeHumidity : mainWateringTime;
    if (!mainWateringStarted) {
        Relay::wateringOn();
        mainWateringStarted = true;
        mainWateringStartedAt = AppTime::getCurrentTime();
        AppBlynk::print("Start main watering for: ");
        AppBlynk::print(wateringTime);
        AppBlynk::println("s");
        return;
    }
    if (AppTime::compareDates(mainWateringStartedAt, AppTime::getCurrentTime()) >= wateringTime) {
        Relay::wateringOff();
        AppBlynk::println("Stop main watering");
        mainWateringStarted = false;
        mainWateringPassed = true;
    }
}

// recheck moisture (6 stage)

void moistureRecheck() {
    if (!waitingBeforeMoistureRecheckPassed || strcmp(wateringStartedFor, "sHumidity") == 0 || moistureRecheckPassed) {
        return;
    }

    int moistureRechecked = getSoilMoisture(wateringStartedFor);
    if (moistureRechecked - moistureStarted > moistureDifferenceThreshold) {
        AppBlynk::print("Moisture recheck success. Difference is: ");
        AppBlynk::println(moistureRechecked - moistureStarted);
        moistureRecheckPassed = true;
    } else {
        AppBlynk::println("Moisture recheck failed!!!");
        AppBlynk::print("Initial moisture: ");
        AppBlynk::println(moistureStarted);
        AppBlynk::print("Rechecked moisture: ");
        AppBlynk::println(moistureRechecked);
        AppBlynk::print("Difference is: ");
        AppBlynk::println(moistureRechecked - moistureStarted);
        Watering::stop();
    }
}

// wait before moisture recheck (5 stage)

void waitBeforeMoistureRecheck() {
    if (!testWateringPassed || strcmp(wateringStartedFor, "sHumidity") == 0 || waitingBeforeMoistureRecheckPassed) {
        return;
    }
    if (!waitingBeforeMoistureRecheckStarted) {
        waitingBeforeMoistureRecheckStarted = true;
        waitingBeforeMoistureRecheckStartedAt = AppTime::getCurrentTime();
        AppBlynk::print("Start waiting before moisture recheck for: ");
        AppBlynk::print(waitingBeforeMoistureRecheckTime);
        AppBlynk::println("s");
        return;
    }
    if (AppTime::compareDates(waitingBeforeMoistureRecheckStartedAt, AppTime::getCurrentTime()) >= waitingBeforeMoistureRecheckTime) {
        AppBlynk::println("Stop waiting before moisture recheck");
        waitingBeforeMoistureRecheckStarted = false;
        waitingBeforeMoistureRecheckPassed = true;
    }
}

// test watering (4 stage)

void testWatering() {
    if (!valveIsOpen || strcmp(wateringStartedFor, "sHumidity") == 0 || testWateringPassed) {
        return;
    }
    if (!testWateringStarted) {
        Relay::wateringOn();
        testWateringStarted = true;
        testWateringStartedAt = AppTime::getCurrentTime();
        AppBlynk::print("Start test watering for: ");
        AppBlynk::print(testWateringTime);
        AppBlynk::println("s");
        return;
    }
    if (AppTime::compareDates(testWateringStartedAt, AppTime::getCurrentTime()) >= testWateringTime) {
        Relay::wateringOff();
        AppBlynk::println("Stop test watering");
        testWateringStarted = false;
        testWateringPassed = true;
    }
}

// open valve (3 stage)

void valve() {
    if (!mixingPassed || valveIsOpen) {
        return;
    }

    if (Relay::wateringValveIsOpen(wateringStartedFor)) {
        valveIsOpen = true;
        AppBlynk::print("Valve was open for: ");
        AppBlynk::println(wateringStartedFor);
        return;
    }

    Relay::wateringOpenValve(wateringStartedFor);
    AppBlynk::print("Try to open valve for: ");
    AppBlynk::println(wateringStartedFor);
}

// water mixing (2 stage)

void mixing() {
    if (mixingPassed) {
        return;
    }
    if (!mixingStarted) {
        Relay::wateringMixingOn();
        mixingStarted = true;
        mixingEnabledAt = AppTime::getCurrentTime();
        AppBlynk::print("Start water mixing for: ");
        AppBlynk::print(mixingTime);
        AppBlynk::println("s");
        return;
    }
    if (AppTime::compareDates(mixingEnabledAt, AppTime::getCurrentTime()) >= mixingTime) {
        Relay::wateringMixingOff();
        AppBlynk::println("Stop water mixing");
        mixingStarted = false;
        mixingPassed = true;
    }
}

// soil moisture check (1 stage)

void soilMoisture() {
    int &wSoilMstrMin = Watering::getIntVariable("wSoilMstrMin");

    const int varsLen = *(&targetVariables + 1) - targetVariables;
    for (int i = 0; i < varsLen; i++) {
        char *targetVarName = targetVariables[i].name;
        int &specialAutoWatering = Watering::getIntVariable(targetVariables[i].autoWateringVar);
        if (specialAutoWatering) {
            strcpy(wateringStartedFor, targetVarName);
            if (strcmp(targetVarName, "sHumidity") != 0) {
                moistureStarted = getSoilMoisture(targetVarName);
            }
            wateringStarted = true;
            break;
        }
    }
    if (!wateringStarted) {
        for (int i = 0; i < varsLen; i++) {
            char *targetVarName = targetVariables[i].name;
            double prevWateringSec = getLastTimeWateringForPot(targetVariables[i].lastWateringVar);
            if (strcmp(targetVarName, "sHumidity") == 0) {
                if (!Sensor::humidityHasWater()) {
                    if (prevWateringSec >= waitBetweenWatering) {
                        strcpy(wateringStartedFor, targetVarName);
                        wateringStarted = true;
                        break;
                    } else {
                        printLastWateringTime(targetVarName, prevWateringSec);
                    }
                }
            } else {
                int potSoilMoisture = getSoilMoisture(targetVarName);
                if (potSoilMoisture < wSoilMstrMin) {
                    if (prevWateringSec >= waitBetweenWatering) {
                        strcpy(wateringStartedFor, targetVarName);
                        moistureStarted = potSoilMoisture;
                        wateringStarted = true;
                        break;
                    } else {
                        printLastWateringTime(targetVarName, prevWateringSec);
                    }
                }
            }
        }
    }

    if (wateringStarted) {
        AppBlynk::print("Start watering for: ");
        AppBlynk::println(wateringStartedFor);
        if (moistureStarted > 0) { // if watering will be started not for the humidifier
            AppBlynk::print("Initial moisture: ");
            AppBlynk::println(moistureStarted);
            AppBlynk::print("Minimum must be: ");
            AppBlynk::println(wSoilMstrMin);
        }
    }
}

// public

void Watering::setVariable(int *var, const char *key) {
    int varsLen = *(&intVariables + 1) - intVariables;
    for (int i = 0; i < varsLen; i++) {
        if (!intVariables[i].key) {
            intVariables[i] = WateringIntVariable(var, key);
            break;
        }
    }
}

void Watering::setVariable(String *var, const char *key) {
    int varsLen = *(&stringVariables + 1) - stringVariables;
    for (int i = 0; i < varsLen; i++) {
        if (!stringVariables[i].key) {
            stringVariables[i] = WateringStringVariable(var, key);
            break;
        }
    }
}

void Watering::stop() {
    Relay::wateringMixingOff();
    Relay::wateringOff();
    Relay::wateringCloseValve(wateringStartedFor);

    const int varsLen = *(&targetVariables + 1) - targetVariables;
    for (int i = 0; i < varsLen; i++) {
        // save time of last watering for certain pot
        if (strcmp(wateringStartedFor, targetVariables[i].name) == 0) {
            String &lastTimeWatering = Watering::getStringVariable(targetVariables[i].lastWateringVar);
            lastTimeWatering = AppTime::getTimeString(AppTime::getCurrentTime());
            AppStorage::putString(targetVariables[i].lastWateringVar, lastTimeWatering);
        }
        // disable special auto watering to do the watering only once
        int &specialWateringEnabled = Watering::getIntVariable(targetVariables[i].autoWateringVar);
        if (specialWateringEnabled) {
            specialWateringEnabled = 0;
            AppBlynk::postDataNoCache(targetVariables[i].autoWateringVar, 0);
        }
    }

    if (wateringStarted) {
        AppBlynk::println("Watering stopped!");
    }

    mixingStarted = false;
    mixingPassed = false;
    testWateringStarted = false;
    testWateringPassed = false;
    valveIsOpen = false;
    waitingBeforeMoistureRecheckStarted = false;
    waitingBeforeMoistureRecheckStartedAt = {0};
    waitingBeforeMoistureRecheckPassed = false;
    moistureRecheckPassed = false;
    mainWateringStarted = false;
    mainWateringStartedAt = {0};
    mainWateringPassed = false;
    memset(wateringStartedFor, 0, sizeof wateringStartedFor);
    moistureStarted = 0;
    wateringStarted = false;
}

void Watering::checkProgress() {
    if (noWaterOrLeakageDetected() || (wateringStarted && Tools::millisOverflowIsClose())) {
        Watering::stop();
        return;
    }
    if (!isWateringEnabled() || !wateringStarted) {
        return;
    }
    mixing();
    valve();
    testWatering();
    waitBeforeMoistureRecheck();
    moistureRecheck();
    mainWatering();
    stopping();
}

void Watering::check() {
    if (
        isWateringEnabled()
        && !Tools::millisOverflowIsClose()
        && !wateringStarted
        && !noWaterOrLeakageDetected()
    ) {
        soilMoisture();
    }
}

