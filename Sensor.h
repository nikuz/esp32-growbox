#ifndef Sensor_h
#define Sensor_h

#include <Arduino.h>

class Sensor {
public:
    Sensor();

    ~Sensor();

    static void parseSerialCommand(const char *command, const char *param);

    static float temperatureGet();

    static bool temperatureMoreThan(int maxValue);

    static bool temperatureLessThan(int minValue);

    static float humidityGet();

    static bool humidityMoreThan(int maxValue);

    static bool humidityLessThan(int minValue);

    static bool humidityHasWater();

    static unsigned int getSoilMoisture(int id, int min, int max);

    static bool wateringHasWater();
};

#endif /* Sensor_h */