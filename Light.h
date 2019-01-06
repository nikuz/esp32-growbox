#ifndef Light_h
#define Light_h

#include <Arduino.h>

struct LightVariable {
    int *var;
    const char *key;

    LightVariable() {}

    LightVariable(int *_var, const char *_key) : var(_var), key(_key) {}
};

class Light {
public:
    Light();

    ~Light();

    static void setVariable(int *var, const char *key);

    static int &getVariable(const char *key);

    static void parseSerialCommand(const char *command, const char *param);

    static int intensity();

    static void setIntensity();
};

#endif /* Light_h */