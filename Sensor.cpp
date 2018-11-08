#include <Arduino.h>
#include <DHT.h>

#include "def.h"
#include "Sensor.h"

static DHT dht(DHTPin, DHTTYPE);

float currentTemperature = 0;
float currentHumidity = 0;

Sensor::Sensor() {}
Sensor::~Sensor() {}

void Sensor::initiate() {
    dht.begin();
    pinMode(HUMIDITY_WATER_SENSOR, INPUT);
}

void Sensor::readDHT() {
    float newTemperature = dht.readTemperature();
    float newHumidity = dht.readHumidity();
	if (isnan(newTemperature) || isnan(newHumidity)) {
		Serial.println("Failed to read from DHT sensor!");
	} else {
		currentTemperature = newTemperature;
		currentHumidity = newHumidity;
	}
}

// temperature

float Sensor::temperatureGet() {
    return currentTemperature;
}

bool Sensor::temperatureMoreThan(int maxValue) {
    float newTemperature = Sensor::temperatureGet();
    return newTemperature > maxValue;
}

bool Sensor::temperatureLessThan(int maxValue) {
    float newTemperature = Sensor::temperatureGet();
    return newTemperature <= maxValue;
}

// humidity

float Sensor::humidityGet() {
    return currentHumidity;
}

bool Sensor::humidityMoreThan(int minValue) {
    float newHumidity = Sensor::humidityGet();
    return newHumidity > minValue;
}

bool Sensor::humidityLessThan(int minValue) {
    float newHumidity = Sensor::humidityGet();
    return newHumidity <= minValue;
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