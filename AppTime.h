#ifndef AppTime_h
#define AppTime_h

#include <Arduino.h>
#include <SimpleTimer.h>

struct AppTimeVariable {
    int *var;
    const char *key;

    AppTimeVariable() {}

    AppTimeVariable(int *_var, const char *_key) : var(_var), key(_key) {}
};

class AppTime {
public:
    AppTime();

    ~AppTime();

    static void obtainSNTP();

    static void parseSerialCommand(const char *command, const char *param);

    static bool localTime(struct tm *timeinfo);

    static void RTCBegin();

    static void RTCUpdateByNtp();

    static int RTCGetTemperature();

    static bool RTCBattery();

    static struct tm RTCGetCurrentTime();

    static struct tm getCurrentTime();

    static int getCurrentHour();

    static int getCurrentMinute();

    static char *getTimeString(struct tm timeStruct, char format[] = "%02u/%02u/%04u %02u:%02u:%02u");

    static void print();

    static void setVariable(int *var, const char *key);

    static int &getVariable(const char *key);

    static int &getLightDayStart();

    static int &getLightDayEnd();

    static bool lightDayDiapasonMatch(int hour);

    static SimpleTimer *getTimer();
};

#endif /* AppTime_h */
