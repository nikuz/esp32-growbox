#include <Arduino.h>

#include "def.h"
#include "Relay.h"
#include "AppSerial.h"
#include "Sensor.h"
#include "AppTime.h"

const char relayOnSerialCommand[] = "rOn";
const char relayOffSerialCommand[] = "rOf";

// light
bool lightEnabled = false;

// ventilation
bool ventilationEnabled = false;
bool ventilationProphylaxisEnabled = false;
const unsigned long ventilationProphylaxisInterval = 60UL * 1000UL * 10UL;  // enable ventilation Prophylaxis every 10 minutes
struct tm ventilationEnableLastTime = {0};

// humidity
bool humidityEnabled = false;

// wind
bool windEnabled = false;

// watering
bool wateringEnabled = false;
bool wateringOpenedValve1 = false;
bool wateringOpenedValve2 = false;
bool wateringOpenedValve3 = false;
bool wateringOpenedValve4 = false;
bool wateringOpenedValveHumidity = false;

Relay::Relay() {}

Relay::~Relay() {}

void Relay::parseSerialCommand(const char *command, const char *param) {
    if (strcmp(command, "rOn") == 0) {
        if (strcmp(param, "light") == 0) {
            lightEnabled = true;
            DEBUG_PRINTLN("Light ON.");
        }
        if (strcmp(param, "vent") == 0) {
            ventilationEnabled = true;
            DEBUG_PRINTLN("Ventilation ON.");
        }
        if (strcmp(param, "humidity") == 0) {
            humidityEnabled = true;
            DEBUG_PRINTLN("Humidity ON.");
        }
        if (strcmp(param, "wind") == 0) {
            windEnabled = true;
            DEBUG_PRINTLN("Wind ON.");
        }
        if (strcmp(param, "wmixing") == 0) {
            wateringEnabled = true;
            DEBUG_PRINTLN("Water mixing ON.");
        }
        if (strcmp(param, "s1") == 0) {
            wateringOpenedValve1 = true;
            DEBUG_PRINTLN("Open valve s1.");
        }
        if (strcmp(param, "s2") == 0) {
            wateringOpenedValve2 = true;
            DEBUG_PRINTLN("Open valve s2.");
        }
        if (strcmp(param, "s3") == 0) {
            wateringOpenedValve3 = true;
            DEBUG_PRINTLN("Open valve s3.");
        }
        if (strcmp(param, "s4") == 0) {
            wateringOpenedValve4 = true;
            DEBUG_PRINTLN("Open valve s4.");
        }
        if (strcmp(param, "sHumidity") == 0) {
            wateringOpenedValveHumidity = true;
            DEBUG_PRINTLN("Open valve sHumidity.");
        }
        if (strcmp(param, "water") == 0) {
            wateringEnabled = true;
            DEBUG_PRINTLN("Watering ON.");
        }
    } else if (strcmp(command, "rOf") == 0) {
        if (strcmp(param, "light") == 0) {
            lightEnabled = false;
            DEBUG_PRINTLN("Light OFF.");
        }
        if (strcmp(param, "vent") == 0) {
            ventilationEnabled = false;
            DEBUG_PRINTLN("Ventilation OFF.");
            ventilationEnableLastTime = AppTime::getCurrentTime();
        }
        if (strcmp(param, "humidity") == 0) {
            humidityEnabled = false;
            DEBUG_PRINTLN("Humidity OFF.");
        }
        if (strcmp(param, "wind") == 0) {
            windEnabled = false;
            DEBUG_PRINTLN("Wind OFF.");
        }
        if (strcmp(param, "s1") == 0) {
            wateringOpenedValve1 = false;
            DEBUG_PRINTLN("Close valve s1.");
        }
        if (strcmp(param, "s2") == 0) {
            wateringOpenedValve2 = false;
            DEBUG_PRINTLN("Close valve s2.");
        }
        if (strcmp(param, "s3") == 0) {
            wateringOpenedValve3 = false;
            DEBUG_PRINTLN("Close valve s3.");
        }
        if (strcmp(param, "s4") == 0) {
            wateringOpenedValve4 = false;
            DEBUG_PRINTLN("Close valve s4.");
        }
        if (strcmp(param, "sHumidity") == 0) {
            wateringOpenedValveHumidity = false;
            DEBUG_PRINTLN("Close valve sHumidity.");
        }
        // do not need to indicate wateringEnabled as false when wmixing off,
        // because watering process is complex, and going in cascade
        // wateringEnabled will be off on water off, this event happen on any watering terminating event
        if (strcmp(param, "water") == 0) {
            wateringEnabled = false;
            DEBUG_PRINTLN("Watering OFF.");
        }
    }
}

// ventilation

bool Relay::isVentilationOn() {
    return ventilationEnabled;
}

bool Relay::isVentilationProphylaxisOn() {
    return ventilationProphylaxisEnabled;
}

void Relay::ventilationOn() {
    SerialFrame ventilationFrame = SerialFrame(relayOnSerialCommand, "vent");
    AppSerial::sendFrame(&ventilationFrame);
}

void Relay::ventilationOff() {
    SerialFrame ventilationFrame = SerialFrame(relayOffSerialCommand, "vent");
    AppSerial::sendFrame(&ventilationFrame);
}

void Relay::ventilationProphylaxis() {
    if (ventilationProphylaxisEnabled) {
        ventilationProphylaxisEnabled = false;
        Relay::ventilationOff();
    } else if (AppTime::compareDates(ventilationEnableLastTime, AppTime::getCurrentTime()) > ventilationProphylaxisInterval) {
        ventilationProphylaxisEnabled = true;
        Relay::ventilationOn();
    }
}

// light

bool Relay::isLightOn() {
    return lightEnabled;
}

void Relay::lightOn() {
    SerialFrame lightFrame = SerialFrame(relayOnSerialCommand, "light");
    AppSerial::sendFrame(&lightFrame);
}

void Relay::lightOff() {
    SerialFrame lightFrame = SerialFrame(relayOffSerialCommand, "light");
    AppSerial::sendFrame(&lightFrame);
}

// humidity

bool Relay::isHumidityOn() {
    return humidityEnabled;
}

void Relay::humidityOn() {
    if (Sensor::humidityHasWater()) {
        SerialFrame humidityFrame = SerialFrame(relayOnSerialCommand, "humidity");
        AppSerial::sendFrame(&humidityFrame);
    }
}

void Relay::humidityOff() {
    SerialFrame humidityFrame = SerialFrame(relayOffSerialCommand, "humidity");
    AppSerial::sendFrame(&humidityFrame);
}

// wind

bool Relay::isWindOn() {
    return windEnabled;
}

void Relay::windOn() {
    SerialFrame windFrame = SerialFrame(relayOnSerialCommand, "wind");
    AppSerial::sendFrame(&windFrame);
}

void Relay::windOff() {
    SerialFrame windFrame = SerialFrame(relayOffSerialCommand, "wind");
    AppSerial::sendFrame(&windFrame);
}

// watering

bool Relay::isWateringOn() {
    return wateringEnabled;
}

void Relay::wateringMixingOn() {
    SerialFrame mixingFrame = SerialFrame(relayOnSerialCommand, "wmixing");
    AppSerial::sendFrame(&mixingFrame);
}

void Relay::wateringMixingOff() {
    SerialFrame mixingFrame = SerialFrame(relayOffSerialCommand, "wmixing");
    AppSerial::sendFrame(&mixingFrame);
}

bool Relay::wateringValveIsOpen(char *valveId) {
    if (strcmp(valveId, "s1") == 0) {
        return wateringOpenedValve1;
    }
    if (strcmp(valveId, "s2") == 0) {
        return wateringOpenedValve2;
    }
    if (strcmp(valveId, "s3") == 0) {
        return wateringOpenedValve3;
    }
    if (strcmp(valveId, "s4") == 0) {
        return wateringOpenedValve4;
    }
    if (strcmp(valveId, "sHumidity") == 0) {
        return wateringOpenedValveHumidity;
    }

    return false;
}

void Relay::wateringOpenValve(char *valveId) {
    SerialFrame openValveFrame = SerialFrame(relayOnSerialCommand, valveId);
    AppSerial::sendFrame(&openValveFrame);
}

void Relay::wateringCloseValve(char *valveId) {
    SerialFrame closeValveFrame = SerialFrame(relayOffSerialCommand, valveId);
    AppSerial::sendFrame(&closeValveFrame);
}

void Relay::wateringOn() {
    SerialFrame waterFrame = SerialFrame(relayOnSerialCommand, "water");
    AppSerial::sendFrame(&waterFrame);
}

void Relay::wateringOff() {
    SerialFrame waterFrame = SerialFrame(relayOffSerialCommand, "water");
    AppSerial::sendFrame(&waterFrame);
}