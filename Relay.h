#ifndef Relay_h
#define Relay_h

#include <Arduino.h>

class Relay {
   public:
    Relay();
    ~Relay();

    void initiate();

    // ventilation
    bool isVentilationOn();
    bool isVentilationProphylaxisOn();
    void ventilationOn();
    void ventilationOff();
    void ventilationProphylaxis();

    // light
    bool isLightOn();
    void lightOn();
    void lightOff();

    // humidity
    bool isHumidityOn();
    void humidityOn();
    void humidityOff();
};

#endif /* Relay_h */