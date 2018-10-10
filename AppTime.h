#ifndef AppTime_h
#define AppTime_h

#include <Arduino.h>

class AppTime {
   public:
    AppTime();
    ~AppTime();

    void obtainSNTP();
    bool localTime(struct tm* timeinfo);

    void RTCBegin();
    void RTCUpdateByNtp();
    int RTCGetTemperature();
    bool RTCBattery();
    bool RTCIsDateTimeValid();
    struct tm RTCGetCurrentTime();

    struct tm getCurrentTime();
    int getCurrentHour();
    String getTimeString(struct tm timeStruct, char format[] = "%02u/%02u/%04u %02u:%02u:%02u");

    void print();
};

#endif /* AppTime_h */
