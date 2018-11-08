#ifndef AppTime_h
#define AppTime_h

#include <Arduino.h>

class AppTime {
   public:
    AppTime();
    ~AppTime();

    static void obtainSNTP();
    static bool localTime(struct tm* timeinfo);

    static void RTCBegin();
    static void RTCUpdateByNtp();
    static int RTCGetTemperature();
    static bool RTCBattery();
    static bool RTCIsDateTimeValid();
    static struct tm RTCGetCurrentTime();

    static struct tm getCurrentTime();
    static int getCurrentHour();
    static String getTimeString(struct tm timeStruct, char format[] = "%02u/%02u/%04u %02u:%02u:%02u");

    static void print();
};

#endif /* AppTime_h */
