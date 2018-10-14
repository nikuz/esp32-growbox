#ifndef Screen_h
#define Screen_h

#include <Arduino.h>

class Screen {
   public:
    Screen();
    ~Screen();

    void initiate();
    void clearBuffer();
    void sendBuffer();
    void printTemperature(int currentTemperature, int currentHumidity);
    void printDayStrip(int currentHour, int lightDayStart, int lightDayEnd);
    void printAppVersion();
    void printTime(struct tm localtime);
    void printHumidityWater(bool hasWater);
};

#endif /* Screen_h */