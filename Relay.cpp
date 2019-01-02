#include <Arduino.h>

#include "def.h"
#include "Relay.h"
#include "AppSerial.h"

const char relayOnSerialCommand[] = "rOn";
const char relayOffSerialCommand[] = "rOf";

// light
bool lightEnabled = false;

// ventilation
bool ventilationEnabled = false;
bool ventilationProphylaxisEnabled = false;
const int ventilationProphylaxisInterval = 60L * 10L;  // enable ventilation Prophylaxis every 10 minutes
unsigned long ventilationEnableLastTime = 0L;

// humidity
bool humidityEnabled = false;

Relay::Relay() {}

Relay::~Relay() {}

void Relay::parseSerialCommand(const char *command, const char *param) {
    if (strcmp(command, "rOn") == 0) {
        if (strcmp(param, "light") == 0) {
            lightEnabled = true;
            Serial.println("Light ON.");
        }
        if (strcmp(param, "vent") == 0) {
            ventilationEnabled = true;
            Serial.println("Ventilation ON.");
        }
        if (strcmp(param, "humidity") == 0) {
            humidityEnabled = true;
            Serial.println("Humidity ON.");
        }
    } else if (strcmp(command, "rOf") == 0) {
        if (strcmp(param, "light") == 0) {
            lightEnabled = false;
            Serial.println("Light OFF.");
        }
        if (strcmp(param, "vent") == 0) {
            ventilationEnabled = false;
            Serial.println("Ventilation OFF.");
            ventilationEnableLastTime = millis();
        }
        if (strcmp(param, "humidity") == 0) {
            humidityEnabled = false;
            Serial.println("Humidity OFF.");
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
    const unsigned long interval = ventilationProphylaxisInterval * 1000L;
    const unsigned long now = millis();
    if (now > interval && now - interval > ventilationEnableLastTime && !ventilationEnabled) {
        ventilationProphylaxisEnabled = true;
        Relay::ventilationOn();
    } else if (ventilationProphylaxisEnabled) {
        ventilationProphylaxisEnabled = false;
        Relay::ventilationOff();
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
	SerialFrame humidityFrame = SerialFrame(relayOnSerialCommand, "humidity");
	AppSerial::sendFrame(&humidityFrame);
}

void Relay::humidityOff() {
	SerialFrame humidityFrame = SerialFrame(relayOffSerialCommand, "humidity");
	AppSerial::sendFrame(&humidityFrame);
}
