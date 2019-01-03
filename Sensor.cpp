#include <Arduino.h>

#include "def.h"
#include "Sensor.h"
#include "Tools.h"

float currentTemperature = 0;
float currentHumidity = 0;
bool humidityWater = false;
bool wateringWater = false;
bool doorOpened = false;
unsigned int soilMoisture[4] = {
        0,
        0,
        0,
        0,
};
unsigned int rainSensors[4] = {
        0,
        0,
        0,
        0,
};

Sensor::Sensor() {}

Sensor::~Sensor() {}

void Sensor::parseSerialCommand(const char *command, const char *param) {
    int value = Tools::StringToUint8(param);
    if (strcmp(command, "temp") == 0) {
        currentTemperature = value;
    }
    if (strcmp(command, "hum") == 0) {
        currentHumidity = value;
    }
    if (strcmp(command, "humw") == 0) {
    	humidityWater = value == 1;
    }
    if (strcmp(command, "water") == 0) {
        wateringWater = value == 1;
    }
    if (strcmp(command, "s1") == 0) {
        soilMoisture[0] = value;
    }
    if (strcmp(command, "s2") == 0) {
        soilMoisture[1] = value;
    }
    if (strcmp(command, "s3") == 0) {
        soilMoisture[2] = value;
    }
    if (strcmp(command, "s4") == 0) {
        soilMoisture[3] = value;
    }
    if (strcmp(command, "rain1") == 0) {
        rainSensors[0] = value;
    }
    if (strcmp(command, "rain2") == 0) {
        rainSensors[1] = value;
    }
    if (strcmp(command, "rain3") == 0) {
        rainSensors[2] = value;
    }
    if (strcmp(command, "rain4") == 0) {
        rainSensors[3] = value;
    }
    if (strcmp(command, "door") == 0) {
        doorOpened = value == 0;
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
	return true;
//    return humidityWater;
}

// soil moisture

unsigned int Sensor::getSoilMoisture(int sensorId, int min, int max) {
    int value = soilMoisture[sensorId - 1];

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

// watering

bool Sensor::wateringHasWater() {
    return wateringWater;
}

bool Sensor::waterLeakageDetected() {
    const int rainSensorsLen = *(&rainSensors + 1) - rainSensors;
    for (int i = 0; i < rainSensorsLen; i++) {
        if (rainSensors[i] == 1) {
            return true;
        }
    }

    return false;
}

bool Sensor::doorIsOpen() {
    return doorOpened;
}