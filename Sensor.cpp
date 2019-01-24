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
static SoilMoistureSensor soilMoistureSensors[] = {
    {SOIL_SENSOR_1, SOIL_SENSOR_1_MIN, SOIL_SENSOR_1_MAX},
    {SOIL_SENSOR_2, SOIL_SENSOR_2_MIN, SOIL_SENSOR_2_MAX},
    {SOIL_SENSOR_3, SOIL_SENSOR_3_MIN, SOIL_SENSOR_3_MAX},
    {SOIL_SENSOR_4, SOIL_SENSOR_4_MIN, SOIL_SENSOR_4_MAX},
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
    int value = atoi(param);
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
    if (strcmp(command, "soil1") == 0) {
        soilMoisture[0] = value;
    }
    if (strcmp(command, "soil2") == 0) {
        soilMoisture[1] = value;
    }
    if (strcmp(command, "soil3") == 0) {
        soilMoisture[2] = value;
    }
    if (strcmp(command, "soil4") == 0) {
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
    return humidityWater;
}

// soil moisture

int Sensor::getSoilMoisture(int sensorId) {
    int value = soilMoisture[sensorId - 1];
    if (value) {
        const int varsLen = *(&soilMoistureSensors + 1) - soilMoistureSensors;
        for (int i = 0; i < varsLen; i++) {
            if (soilMoistureSensors[i].sensorId == sensorId) {
                value = map(value, soilMoistureSensors[i].min, soilMoistureSensors[i].max, 0, 100);
                if (value < 0) {
                    value = 0;
                } else if (value > 100) {
                    value = 100;
                }
            }
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