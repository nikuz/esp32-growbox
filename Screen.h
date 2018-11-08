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
    static void printTemperature(int currentTemperature, int currentHumidity);
    static void printDayStrip(int currentHour, int lightDayStart, int lightDayEnd);
    static void printAppVersion();
    static void printTime(struct tm localtime);
    static void printHumidityWater(bool hasWater);
};

#endif /* Screen_h */