#include <Arduino.h>

#include "def.h"
#include "Relay.h"
#include "Blynk.h"

static Blynk blynkClient;

Relay::Relay() {}
Relay::~Relay() {}

void Relay::initiate() {
    pinMode(RELAY_1, OUTPUT);
    digitalWrite(RELAY_1, LOW);
    pinMode(RELAY_2, OUTPUT);
    digitalWrite(RELAY_2, LOW);
}

// ventilation
bool ventilationEnabled = false;
bool ventilationProphylaxisEnabled = false;
const int ventilationProphylaxisInterval = 60 * 10;  // enable ventilation Prophylaxis every 10 minutes
unsigned long ventilationEnableLastTime = millis();

bool Relay::isVentilationOn() {
    return ventilationEnabled;
}

bool Relay::isVentilationProphylaxisOn() {
    return ventilationProphylaxisEnabled;
}

void Relay::ventilationOn() {
    if (!ventilationEnabled) {
        blynkClient.terminal("Ventilation ON.");
        digitalWrite(RELAY_2, HIGH);
        ventilationEnabled = true;
    }
}

void Relay::ventilationOff() {
    if (ventilationEnabled) {
        blynkClient.terminal("Ventilation OFF.");
        digitalWrite(RELAY_2, LOW);
        ventilationEnabled = false;
        ventilationEnableLastTime = millis();
    }
}

void Relay::ventilationProphylaxis() {
    const unsigned long interval = ventilationProphylaxisInterval * 1000L;
    const unsigned long now = millis();
    if (now - interval > 0 && now - interval > ventilationEnableLastTime && !ventilationEnabled) {
        ventilationProphylaxisEnabled = true;
        this->ventilationOn();
    } else if (ventilationProphylaxisEnabled) {
        ventilationProphylaxisEnabled = false;
        this->ventilationOff();
    }
}

// light
bool lightEnabled = false;

bool Relay::isLightOn() {
    return lightEnabled;
}

void Relay::lightOn() {
    if (!lightEnabled) {
        digitalWrite(RELAY_1, HIGH);
        lightEnabled = true;
    }
}

void Relay::lightOff() {
    if (lightEnabled) {
        digitalWrite(RELAY_1, LOW);
        lightEnabled = false;
    }
}
