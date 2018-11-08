#ifndef Relay_h
#define Relay_h

#include <Arduino.h>

class Relay {
public:
    Relay();

    ~Relay();

    static void initiate();

    // ventilation
    static bool isVentilationOn();

    static bool isVentilationProphylaxisOn();

    static void ventilationOn();

    static void ventilationOff();

    static void ventilationProphylaxis();

    // light
    static bool isLightOn();

    static void lightOn();

    static void lightOff();

    // humidity
    static bool isHumidityOn();

    static void humidityOn();

    static void humidityOff();
};

#endif /* Relay_h */