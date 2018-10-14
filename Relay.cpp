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
    pinMode(RELAY_3, OUTPUT);
    digitalWrite(RELAY_3, LOW);
}

// ventilation
bool ventilationEnabled = false;
bool ventilationProphylaxisEnabled = false;
const int ventilationProphylaxisInterval = 60 * 10;  // enable ventilation Prophylaxis every 10 minutes
unsigned long ventilationEnableLastTime = 0;

bool Relay::isVentilationOn() {
    return ventilationEnabled;
}

bool Relay::isVentilationProphylaxisOn() {
    return ventilationProphylaxisEnabled;
}

void Relay::ventilationOn() {
    if (!ventilationEnabled) {
        digitalWrite(RELAY_2, HIGH);
        ventilationEnabled = true;
        blynkClient.terminal("Ventilation ON.");
    }
}

void Relay::ventilationOff() {
    if (ventilationEnabled) {
        digitalWrite(RELAY_2, LOW);
        ventilationEnabled = false;
        ventilationEnableLastTime = millis();
        blynkClient.terminal("Ventilation OFF.");
    }
}

void Relay::ventilationProphylaxis() {
    const unsigned long interval = ventilationProphylaxisInterval * 1000L;
    const unsigned long now = millis();
    if (now > interval && now - interval > ventilationEnableLastTime && !ventilationEnabled) {
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

// humidity
bool humidityEnabled = false;

bool Relay::isHumidityOn() {
    return humidityEnabled;
}

void Relay::humidityOn() {
    if (!humidityEnabled) {
        digitalWrite(RELAY_3, HIGH);
        humidityEnabled = true;
        blynkClient.terminal("Humidity ON.");
    }
}

void Relay::humidityOff() {
    if (humidityEnabled) {
        digitalWrite(RELAY_3, LOW);
        humidityEnabled = false;
        blynkClient.terminal("Humidity OFF.");
    }
}
