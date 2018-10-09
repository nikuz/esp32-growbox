#ifndef AppDHT_h
#define AppDHT_h

#include <Arduino.h>

class AppDHT {
   public:
    AppDHT();
    ~AppDHT();

    void initiate();
    
    void temperatureRead();
    float temperatureGet();
    bool temperatureMoreThan(int maxValue);
    bool temperatureLessThan(int minValue);
    
    void humidityRead();
    float humidityGet();
    bool humidityMoreThan(int maxValue);
    bool humidityLessThan(int minValue);

    void read();
};

#endif /* AppDHT_h */