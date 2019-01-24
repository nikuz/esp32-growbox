#ifndef Screen_h
#define Screen_h

#include <Arduino.h>

class Screen {
public:
    Screen();

    ~Screen();

    static void initiate();

    static void clearBuffer();

    static void sendBuffer();

    static void printTemperature(float currentTemperature, float currentHumidity);

    static void printDayStrip(int currentHour, int lightDayStart, int lightDayEnd);

    static void printAppVersion();

    static void printTime(struct tm localtime);

    static void printUptime();

    static void printHumidityWater(bool hasWater);

    static void printWater(bool hasWater);

    static void printWaterLeakage(bool leakageDetected);

    static void refresh();
};

#endif /* Screen_h */