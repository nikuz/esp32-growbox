#include <Arduino.h>
#include <DHT.h>

#include "def.h"
#include "AppDHT.h"
#include "Blynk.h"

static Blynk blynkClient;

static DHT dht(DHTPin, DHTTYPE);

float currentTemperature = 0;
float currentHumidity = 0;

AppDHT::AppDHT() {}
AppDHT::~AppDHT() {}

void AppDHT::initiate() {
    dht.begin();
}

// temperature

void AppDHT::temperatureRead() {
    float newTemperature = dht.readTemperature();
    if (isnan(newTemperature)) {
        blynkClient.terminal("Failed to read from DHT sensor!");
    } else if (currentTemperature != newTemperature) {
        currentTemperature = newTemperature;
    }
}

float AppDHT::temperatureGet() {
    return currentTemperature;
}

bool AppDHT::temperatureMoreThan(int maxValue) {
    float newTemperature = this->temperatureGet();
    return newTemperature > maxValue;
}

bool AppDHT::temperatureLessThan(int maxValue) {
    float newTemperature = this->temperatureGet();
    return newTemperature <= maxValue;
}

// humidity

float AppDHT::humidityGet() {
    return currentHumidity;
}

void AppDHT::humidityRead() {
    float newHumidity = dht.readHumidity();
    if (isnan(newHumidity)) {
        blynkClient.terminal("Failed to read from DHT sensor!");
    } else if (currentHumidity != newHumidity) {
        currentHumidity = newHumidity;
    }
}

bool AppDHT::humidityMoreThan(int minValue) {
    float newHumidity = this->humidityGet();
    return newHumidity > minValue;
}

bool AppDHT::humidityLessThan(int minValue) {
    float newHumidity = this->humidityGet();
    return newHumidity <= minValue;
}

void AppDHT::read() {
    this->temperatureRead();
    this->humidityRead();
}