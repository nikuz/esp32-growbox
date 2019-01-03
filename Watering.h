#ifndef Watering_h
#define Watering_h

#include <Arduino.h>

class Watering {
public:
    Watering();

    ~Watering();

    static void check(int &wSoilMstrMin);
};

#endif /* Watering_h */