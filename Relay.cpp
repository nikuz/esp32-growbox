#include <Arduino.h>

#include "def.h"
#include "Relay.h"

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
const int ventilationProphylaxisInterval = 60L * 10L;  // enable ventilation Prophylaxis every 10 minutes
unsigned long ventilationEnableLastTime = 0L;

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
        Serial.println("Ventilation ON.");
    }
}

void Relay::ventilationOff() {
    if (ventilationEnabled) {
        digitalWrite(RELAY_2, LOW);
        ventilationEnabled = false;
        ventilationEnableLastTime = millis();
        Serial.println("Ventilation OFF.");
    }
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
bool lightEnabled = false;

bool Relay::isLightOn() {
    return lightEnabled;
}

void Relay::lightOn() {
    if (!lightEnabled) {
        Serial.println("Light ON.");
        digitalWrite(RELAY_1, HIGH);
        lightEnabled = true;
    }
}

void Relay::lightOff() {
    if (lightEnabled) {
        Serial.println("Light OFF.");
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
        Serial.println("Humidity ON.");
    }
}

void Relay::humidityOff() {
    if (humidityEnabled) {
        digitalWrite(RELAY_3, LOW);
        humidityEnabled = false;
        Serial.println("Humidity OFF.");
    }
}
