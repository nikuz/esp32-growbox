#ifndef Light_h
#define Light_h

#include <Arduino.h>

class Light {
public:
    Light();

    ~Light();

    static void parseSerialCommand(const char *command, const char *param);

    static int intensity();

    static void setIntensity(int &lightDayStart, int &lightDayEnd, int &lightMaxInt);
};

#endif /* Light_h */