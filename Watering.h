#ifndef Watering_h
#define Watering_h

#include <Arduino.h>

struct WateringVariable {
    int *var;
    const char *key;

    WateringVariable() {}

    WateringVariable(int *_var, const char *_key) : var(_var), key(_key) {}
};

class Watering {
public:
    Watering();

    ~Watering();

    static void setVariable(int *var, const char *key);

    static int &getVariable(const char *key);

    static void initiate();

    static void check();
};

#endif /* Watering_h */