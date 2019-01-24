#ifndef Sensor_h
#define Sensor_h

#include <Arduino.h>

struct SoilMoistureSensor {
    int sensorId;
    int min;
    int max;
};

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

    static int getSoilMoisture(int sensorId);

    static bool wateringHasWater();

    static bool waterLeakageDetected();

    static bool doorIsOpen();
};

#endif /* Sensor_h */