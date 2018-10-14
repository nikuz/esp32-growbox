#ifndef Sensor_h
#define Sensor_h

#include <Arduino.h>

class Sensor {
   public:
    Sensor();
    ~Sensor();

    void initiate();
    
    void temperatureRead();
    float temperatureGet();
    bool temperatureMoreThan(int maxValue);
    bool temperatureLessThan(int minValue);
    
    void humidityRead();
    float humidityGet();
    bool humidityMoreThan(int maxValue);
    bool humidityLessThan(int minValue);

    void readDHT();

    bool humidityHasWater();
    unsigned int getSoilMoisture(int sensorId, int min, int max);
};

#endif /* Sensor_h */