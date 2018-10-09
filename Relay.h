#ifndef Relay_h
#define Relay_h

#include <Arduino.h>

class Relay {
   public:
    Relay();
    ~Relay();

    void initiate();

    bool isVentilationOn();
    bool isVentilationProphylaxisOn();
    void ventilationOn();
    void ventilationOff();
    void ventilationProphylaxis();

    bool isLightOn();
    void lightOn();
    void lightOff();
};

#endif /* Relay_h */