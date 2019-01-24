#ifndef AppTime_h
#define AppTime_h

#include <Arduino.h>
#include <Time.h>

typedef void (*timer_callback)(void);

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

    static struct tm *RTCGetCurrentTime();

    static struct tm getCurrentTime();

    static int getCurrentHour();

    static int getCurrentMinute();

    static char *getTimeString(struct tm timeStruct, char format[] = "%02u/%02u/%04u %02u:%02u:%02u");

    static struct tm getTmFromString(const char *value);

    static double compareDates(tm date1, tm date2);

    static double compareDates(String date1, tm date2);

    static void print();

    static void setVariable(int *var, const char *key);

    static int &getVariable(const char *key);

    static int &getLightDayStart();

    static int &getLightDayEnd();

    static bool lightDayDiapasonMatch(int hour);

    static int getOverFlowCounter();

    // maximum number of timers
    const static int MAX_TIMERS = 16;

    // setTimer() constants
    const static int RUN_FOREVER = 0;
    const static int RUN_ONCE = 1;

    // this function must be called inside loop()
    void run();

    // call function f every d milliseconds
    int setInterval(long d, timer_callback f);

private:
    // find the first available slot
    int findFirstFreeSlot();

    // value returned by the millis() function
    // in the previous run() call
    unsigned long prev_millis[MAX_TIMERS];

    // pointers to the callback functions
    timer_callback callbacks[MAX_TIMERS];

    // delay values
    long delays[MAX_TIMERS];

    // actual number of timers in use
    int numTimers;
};

#endif /* AppTime_h */
