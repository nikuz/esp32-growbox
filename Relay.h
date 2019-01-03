#ifndef Relay_h
#define Relay_h

#include <Arduino.h>

class Relay {
public:
    Relay();

    ~Relay();

    static void parseSerialCommand(const char *command, const char *param);

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

    // wind
    static bool isWindOn();

    static void windOn();

    static void windOff();

    // watering
    static bool isWateringOn();

    static void wateringMixingOn();

    static void wateringMixingOff();

    static bool wateringValveIsOpen(char *valveId);

    static void wateringOpenValve(char *valveId);

    static void wateringCloseValve(char *valveId);

    static void wateringOn();

    static void wateringOff();
};

#endif /* Relay_h */