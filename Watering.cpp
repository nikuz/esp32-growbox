#include <Arduino.h>

#include "def.h"
#include "Watering.h"
#include "Tools.h"
#include "Sensor.h"
#include "Relay.h"

int wateringInterval = 5;  // check every 5 seconds
unsigned long wateringLastTime = 0;
int wateringProgressCheckInterval = 1;  // check every second
unsigned long wateringProgressCheckLastTime = 0;

bool wateringStarted = false;
char wateringStartedFor[10];

bool mainWateringStarted = false;
unsigned long mainWateringStartedAt = 0;
unsigned long mainWateringTime = 10L * 1000L;           // 10 sec
unsigned long mainWateringTimeHumidity = 30L * 1000L;   // 30 sec
bool mainWateringPassed = false;

bool moistureRecheckPassed = false;
unsigned int moistureDifferenceThreshold = 10;
unsigned int moistureRechecked = 0;

bool waitingBeforeMoistureRecheckStarted = false;
unsigned long waitingBeforeMoistureRecheckStartedAt = 0;
unsigned long waitingBeforeMoistureRecheckTime = 10L * 1000L;
bool waitingBeforeMoistureRecheckPassed = false;

bool testWateringStarted = false;
unsigned long testWateringStartedAt = 0;
unsigned long testWateringTime = 5L * 1000L;
bool testWateringPassed = false;

bool valveIsOpen = false;

bool mixingStarted = false;
unsigned long mixingEnabledAt = 0;
unsigned long mixingTime = 30L * 1000L;
bool mixingPassed = false;

unsigned int moistureStarted = 0;

Watering::Watering() {}

Watering::~Watering() {}

void stop() {
    Relay::wateringMixingOff();
    Relay::wateringOff();
    Relay::wateringCloseValve(wateringStartedFor);
    mixingStarted = false;
    mixingPassed = false;
    testWateringStarted = false;
    testWateringPassed = false;
    valveIsOpen = false;
    waitingBeforeMoistureRecheckStarted = false;
    waitingBeforeMoistureRecheckStartedAt = 0;
    waitingBeforeMoistureRecheckPassed = false;
    mainWateringStarted = false;
    mainWateringStartedAt = 0;
    mainWateringPassed = false;
    memset(wateringStartedFor, 0, sizeof wateringStartedFor);
    moistureRechecked = 0;
    moistureStarted = 0;
    wateringStarted = false;

    Serial.println("Watering stopped!");
    Serial.println();
}

unsigned int getSoilMoisture(const char *sensorId) {
    if (strcmp(sensorId, "s1") == 0) {
        return Sensor::getSoilMoisture(SOIL_SENSOR_1, SOIL_SENSOR_1_MIN, SOIL_SENSOR_1_MAX);
    }
    if (strcmp(sensorId, "s2") == 0) {
        return Sensor::getSoilMoisture(SOIL_SENSOR_2, SOIL_SENSOR_2_MIN, SOIL_SENSOR_2_MAX);
    }
    if (strcmp(sensorId, "s3") == 0) {
        return Sensor::getSoilMoisture(SOIL_SENSOR_3, SOIL_SENSOR_3_MIN, SOIL_SENSOR_3_MAX);
    }
    if (strcmp(sensorId, "s4") == 0) {
        return Sensor::getSoilMoisture(SOIL_SENSOR_4, SOIL_SENSOR_4_MIN, SOIL_SENSOR_4_MAX);
    }

    return 0;
}

bool noWaterOrLeakageDetected() {
    if (!Sensor::wateringHasWater()) {
        Serial.println("No water. Watering terminated!!!");
        stop();
        return true;
    }
    if (Sensor::waterLeakageDetected()) {
        Serial.println("Water leakage detected. Watering terminated!!!");
        return true;
    }

    return false;
}

// stopping (8 stage)

void stopping() {
    if (!wateringStarted) {
        return;
    }

    if (mainWateringPassed) {
        stop();
    }
}

// main watering (7 stage)

void mainWatering() {
    if (!wateringStarted || !valveIsOpen || mainWateringPassed) {
        return;
    }
    const bool wateringForHumidity = strcmp(wateringStartedFor, "sHumidity") == 0;
    if (!wateringForHumidity && !moistureRecheckPassed) {
        return;
    }
    if (noWaterOrLeakageDetected()) {
        stop();
        return;
    }
    if (!mainWateringStarted) {
        Relay::wateringOn();
        mainWateringStarted = true;
        mainWateringStartedAt = millis();
        Serial.print("Start main watering for: ");
        Serial.print(mainWateringTime / 1000L);
        Serial.println("s");
        return;
    }
    unsigned long wateringTime = wateringForHumidity ? mainWateringTimeHumidity : mainWateringTime;
    if (millis() > mainWateringStartedAt + wateringTime) {
        Relay::wateringOff();
        Serial.println("Stop main watering");
        mainWateringStarted = false;
        mainWateringPassed = true;
    }
}

// recheck moisture (6 stage)

void moistureRecheck() {
    if (
        !wateringStarted
        || !waitingBeforeMoistureRecheckPassed
        || strcmp(wateringStartedFor, "sHumidity") == 0
        || moistureRecheckPassed
    ) {
        return;
    }

    moistureRechecked = getSoilMoisture(wateringStartedFor);
    if (moistureRechecked - moistureStarted > moistureDifferenceThreshold) {
        moistureRecheckPassed = true;
    } else {
        Serial.println("Moisture recheck failed!!!");
        Serial.print("Initial moisture: ");
        Serial.println(moistureStarted);
        Serial.print("Rechecked moisture: ");
        Serial.println(moistureRechecked);
        Serial.print("Difference is: ");
        Serial.println(moistureRechecked - moistureStarted);
        stop();
    }
}

// wait before moisture recheck (5 stage)

void waitBeforeMoistureRecheck() {
    if (
        !wateringStarted
        || !testWateringPassed
        || strcmp(wateringStartedFor, "sHumidity") == 0
        || waitingBeforeMoistureRecheckPassed
    ) {
        return;
    }
    if (!waitingBeforeMoistureRecheckStarted) {
        waitingBeforeMoistureRecheckStarted = true;
        waitingBeforeMoistureRecheckStartedAt = millis();
        Serial.print("Start waiting before moisture recheck for: ");
        Serial.print(waitingBeforeMoistureRecheckTime / 1000L);
        Serial.println("s");
        return;
    }
    if (millis() > waitingBeforeMoistureRecheckStartedAt + waitingBeforeMoistureRecheckTime) {
        Serial.println("Stop waiting before moisture recheck");
        waitingBeforeMoistureRecheckStarted = false;
        waitingBeforeMoistureRecheckPassed = true;
    }
}

// test watering (4 stage)

void testWatering() {
    if (
        !wateringStarted
        || !valveIsOpen
        || strcmp(wateringStartedFor, "sHumidity") == 0
        || testWateringPassed
    ) {
        return;
    }
    if (noWaterOrLeakageDetected()) {
        stop();
        return;
    }
    if (!testWateringStarted) {
        Relay::wateringOn();
        testWateringStarted = true;
        testWateringStartedAt = millis();
        Serial.print("Start test watering for: ");
        Serial.print(testWateringTime / 1000L);
        Serial.println("s");
        return;
    }
    if (millis() > testWateringStartedAt + testWateringTime) {
        Relay::wateringOff();
        Serial.println("Stop test watering");
        testWateringStarted = false;
        testWateringPassed = true;
    }
}

// open valve (3 stage)

void valve() {
    if (!wateringStarted || !mixingPassed || valveIsOpen) {
        return;
    }

    if (Relay::wateringValveIsOpen(wateringStartedFor)) {
        valveIsOpen = true;
        Serial.print("Valve was open for: ");
        Serial.println(wateringStartedFor);
        return;
    }

    Relay::wateringOpenValve(wateringStartedFor);
    Serial.print("Try to open valve for: ");
    Serial.println(wateringStartedFor);
}

// water mixing (2 stage)

void mixing() {
    if (!wateringStarted || mixingPassed) {
        return;
    }
    if (noWaterOrLeakageDetected()) {
        stop();
        return;
    }
    if (!mixingStarted) {
        Relay::wateringMixingOn();
        mixingStarted = true;
        mixingEnabledAt = millis();
        Serial.print("Start water mixing for: ");
        Serial.print(mixingTime / 1000L);
        Serial.println("s");
        return;
    }
    if (millis() > mixingEnabledAt + mixingTime) {
        Relay::wateringMixingOff();
        Serial.println("Stop water mixing");
        mixingStarted = false;
        mixingPassed = true;
    }
}

// soil moisture check (1 stage)

void soilMoisture(int &wSoilMstrMin) {
    if (wateringStarted || noWaterOrLeakageDetected()) {
        return;
    }
    const char* firstPot = "s1";
    unsigned int firstPotMoisture = getSoilMoisture(firstPot);
    if (firstPotMoisture < wSoilMstrMin) {
        strcpy(wateringStartedFor, firstPot);
        moistureStarted = firstPotMoisture;
    }
    const char* secondPot = "s2";
    unsigned int secondPotMoisture = getSoilMoisture(secondPot);
    if (secondPotMoisture < wSoilMstrMin) {
        strcpy(wateringStartedFor, secondPot);
        moistureStarted = secondPotMoisture;
    }
    const char* thirdPot = "s3";
    unsigned int thirdPotMoisture = getSoilMoisture(thirdPot);
    if (thirdPotMoisture < wSoilMstrMin) {
        strcpy(wateringStartedFor, thirdPot);
        moistureStarted = thirdPotMoisture;
    }
    const char* fourthPot = "s4";
    unsigned int fourthPotMoisture = getSoilMoisture(fourthPot);
    if (fourthPotMoisture < wSoilMstrMin) {
        strcpy(wateringStartedFor, fourthPot);
        moistureStarted = fourthPotMoisture;
    }
    if (!Sensor::humidityHasWater()) {
        strcpy(wateringStartedFor, "sHumidity");
    }

    if (
        strcmp(wateringStartedFor, "s1") == 0
        || strcmp(wateringStartedFor, "s2") == 0
        || strcmp(wateringStartedFor, "s3") == 0
        || strcmp(wateringStartedFor, "s4") == 0
        || strcmp(wateringStartedFor, "sHumidity") == 0
    ) {
        Serial.print("Start watering for: ");
        Serial.println(wateringStartedFor);
        if (moistureStarted > 0) {
            Serial.print("Initial moisture: ");
            Serial.println(moistureStarted);
            Serial.print("Minimum must be: ");
            Serial.println(wSoilMstrMin);
        }
        wateringStarted = true;
    }
}

// public

void Watering::check(int &wSoilMstrMin) {
    if (Tools::timerCheck(wateringInterval, wateringLastTime)) {
        soilMoisture(wSoilMstrMin);
        wateringLastTime = millis();
    }
    if (Tools::timerCheck(wateringProgressCheckInterval, wateringProgressCheckLastTime)) {
        mixing();
        valve();
        testWatering();
        waitBeforeMoistureRecheck();
        moistureRecheck();
        mainWatering();
        stopping();
        wateringProgressCheckLastTime = millis();
    }
}

