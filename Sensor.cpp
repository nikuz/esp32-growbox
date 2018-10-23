#include <Arduino.h>
#include <DHT.h>

#include "def.h"
#include "Sensor.h"
#include "Blynk.h"

static Blynk blynkClient;

static DHT dht(DHTPin, DHTTYPE);

float currentTemperature = 0;
float currentHumidity = 0;

Sensor::Sensor() {}
Sensor::~Sensor() {}

void Sensor::initiate() {
    dht.begin();
    pinMode(HUMIDITY_WATER_SENSOR, INPUT);
}

// temperature

void Sensor::temperatureRead() {
    float newTemperature = dht.readTemperature();
    if (isnan(newTemperature)) {
        blynkClient.terminal("Failed to read from DHT sensor!");
    } else if (currentTemperature != newTemperature) {
        currentTemperature = newTemperature;
    }
}

float Sensor::temperatureGet() {
    return currentTemperature;
}

bool Sensor::temperatureMoreThan(int maxValue) {
    float newTemperature = this->temperatureGet();
    return newTemperature > maxValue;
}

bool Sensor::temperatureLessThan(int maxValue) {
    float newTemperature = this->temperatureGet();
    return newTemperature <= maxValue;
}

// humidity

float Sensor::humidityGet() {
    return currentHumidity;
}

void Sensor::humidityRead() {
    float newHumidity = dht.readHumidity();
    if (isnan(newHumidity)) {
        blynkClient.terminal("Failed to read from DHT sensor!");
    } else if (currentHumidity != newHumidity) {
        currentHumidity = newHumidity;
    }
}

bool Sensor::humidityMoreThan(int minValue) {
    float newHumidity = this->humidityGet();
    return newHumidity > minValue;
}

bool Sensor::humidityLessThan(int minValue) {
    float newHumidity = this->humidityGet();
    return newHumidity <= minValue;
}

void Sensor::readDHT() {
    this->temperatureRead();
    this->humidityRead();
}

bool Sensor::humidityHasWater() {
    int hasWater = digitalRead(HUMIDITY_WATER_SENSOR);
    return hasWater ? true : false;
}

unsigned int Sensor::getSoilMoisture(int sensorId, int min, int max) {
    int value = analogRead(sensorId);
    
    if (value) {
        value = map(value, min, max, 0, 100);
        if (value < 0) {
            value = 0;
        } else if (value > 100) {
            value = 100;
        }
    }

    return value;
}