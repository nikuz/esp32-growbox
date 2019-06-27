#include <Arduino.h>
#include <Time.h>

#include "def.h"
#include "AppTime.h"
#include "AppWiFi.h"
#include "AppBlynk.h"
#include "Tools.h"

const char *ntpServer = "1.rs.pool.ntp.org";
const char *ntpServer2 = "0.europe.pool.ntp.org";
const char *ntpServer3 = "1.europe.pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 0;
static AppTimeVariable variables[2];
static int blankVariable = -1;

struct tm NTPCurrentTime = {
        tm_sec: -1,
        tm_min: -1,
        tm_hour: -1,
        tm_mday: -1,
        tm_mon: -1,
        tm_year: -1,
        tm_wday: -1,
        tm_yday: -1,
        tm_isdst: -1
};
struct tm RTCCurrentTime = {
        tm_sec: -1,
        tm_min: -1,
        tm_hour: -1,
        tm_mday: -1,
        tm_mon: -1,
        tm_year: -1,
        tm_wday: -1,
        tm_yday: -1,
        tm_isdst: -1
};
int rtcTemperature = 0;
bool rtcBatteryAlive = true;

static inline unsigned long elapsed() { return millis(); }
int overFlowCounter;

AppTime::AppTime() {
    unsigned long current_millis = elapsed();

    for (int i = 0; i < MAX_TIMERS; i++) {
        callbacks[i] = 0;                   // if the callback pointer is zero, the slot is free, i.e. doesn't "contain" any timer
        prev_millis[i] = current_millis;
        delays[i] = 0;
        names[i] = "";
    }

    numTimers = 0;
    overFlowCounter = 0;
}

AppTime::~AppTime() {}

void AppTime::obtainSNTP() {
    if (AppWiFi::isConnected()) {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, ntpServer2, ntpServer3);
    }
}

void AppTime::parseSerialCommand(const char *command, const char *param) {
    if (strcmp(command, "time") == 0) {
        RTCCurrentTime = AppTime::getTmFromString(param);
    }
    if (strcmp(command, "ttemp") == 0) {
        int value = atoi(param);
        rtcTemperature = value;
    }
    if (strcmp(command, "tbtr") == 0) {
        int value = atoi(param);
        rtcBatteryAlive = value == 1;
    }
}

bool AppTime::localTime(struct tm *timeinfo) {
    return getLocalTime(timeinfo);
}

int AppTime::RTCGetTemperature() {
    return rtcTemperature;
}

bool AppTime::RTCBattery() {
    return rtcBatteryAlive;
}

struct tm *AppTime::RTCGetCurrentTime() {
    return &RTCCurrentTime;
}

struct tm AppTime::getCurrentTime() {
    struct tm timeinfo = NTPCurrentTime;
    if (!AppWiFi::isConnected() || !AppTime::localTime(&timeinfo)) {
        if (AppTime::RTCBattery()) {
            timeinfo = RTCCurrentTime;
        } else {
            // AppTime::RTCBegin();
            // AppTime::RTCUpdateByNtp();
        }
    }

    return timeinfo;
}

int AppTime::getCurrentHour() {
    struct tm currentTime = AppTime::getCurrentTime();
    return currentTime.tm_hour;
}

int AppTime::getCurrentMinute() {
    struct tm currentTime = AppTime::getCurrentTime();
    return currentTime.tm_min;
}

char *AppTime::getTimeString(struct tm timeStruct, char format[]) {
    static char timeString[20];
    snprintf_P(
            timeString,
            sizeof timeString,
            PSTR(format),
            timeStruct.tm_mon + 1,
            timeStruct.tm_mday,
            timeStruct.tm_year + 1900,
            timeStruct.tm_hour,
            timeStruct.tm_min,
            timeStruct.tm_sec
    );
    return timeString;
}

struct tm AppTime::getTmFromString(const char *value) {
    // 10/10/2018 00:09:21
    int month = -1;
    int day = -1;
    int year = -1;
    int hours = -1;
    int minutes = -1;
    int seconds = -1;

    char dateString[20];
    strncpy(dateString, value, 20);

    char *pch;
    const char *delimiter = " /:";
    pch = strtok(dateString, delimiter);
    int i = 0;
    while (pch != NULL) {
        switch (i) {
            case 0:
                month = atoi(pch);
                break;
            case 1:
                day = atoi(pch);
                break;
            case 2:
                year = atoi(pch);
                break;
            case 3:
                hours = atoi(pch);
                break;
            case 4:
                minutes = atoi(pch);
                break;
            case 5:
                seconds = atoi(pch);
                break;
        }
        pch = strtok(NULL, delimiter);
        i++;
    }

    struct tm dateTime = {
            tm_sec: seconds,
            tm_min: minutes,
            tm_hour: hours,
            tm_mday: day,
            tm_mon: month - 1, // 0 based
            tm_year: year - 1900,
            tm_wday: 0,
            tm_yday: 0,
            tm_isdst: -1
    };

    return dateTime;
}

double AppTime::compareDates(tm date1, tm date2) {
    return difftime(mktime(&date2), mktime(&date1));
}

double AppTime::compareDates(String date1, tm date2) {
    struct tm date1Struct = getTmFromString(date1.c_str());
    return difftime(mktime(&date2), mktime(&date1Struct));
}

void AppTime::print() {
    struct tm timeinfo = {0};
    AppTime::localTime(&timeinfo);

    char *ntpTime[] = {"ntpTime: ", AppTime::getTimeString(timeinfo)};
    char *ntpTimeStr = Tools::getCharArray(ntpTime, 2);
    AppBlynk::println(ntpTimeStr);

    char *rtcTime[] = {"rtcTime: ", AppTime::getTimeString(RTCCurrentTime)};
    char *rtcTimeStr = Tools::getCharArray(rtcTime, 2);
    AppBlynk::println(rtcTimeStr);
}

void AppTime::setVariable(int *var, const char *key) {
    int varsLen = *(&variables + 1) - variables;
    for (int i = 0; i < varsLen; i++) {
        if (!variables[i].key) {
            variables[i] = AppTimeVariable(var, key);
            break;
        }
    }
}

int &AppTime::getVariable(const char *key) {
    const int varsLen = *(&variables + 1) - variables;
    for (int i = 0; i < varsLen; i++) {
        if (variables[i].key == key) {
            return *variables[i].var;
        }
    }

    return blankVariable;
}

int &AppTime::getLightDayStart() {
    return getVariable("lightDayStart");
}

int &AppTime::getLightDayEnd() {
    return getVariable("lightDayEnd");
}

bool AppTime::lightDayDiapasonMatch(int hour) {
    int &lightDayStart = getLightDayStart();
    int &lightDayEnd = getLightDayEnd();
    if (lightDayStart > lightDayEnd) {
        return hour >= lightDayStart || hour < lightDayEnd;
    }
    return hour >= lightDayStart && hour < lightDayEnd;
}

void AppTime::run() {
    int i;
    unsigned long current_millis;
    bool overflowIncreased = false;

    // get current time
    current_millis = elapsed();

    for (i = 0; i < MAX_TIMERS; i++) {
        // no callback == no timer, i.e. jump over empty slots
        if (callbacks[i]) {
            // is it time to process this timer ?
            // see http://arduino.cc/forum/index.php/topic,124048.msg932592.html#msg932592
            if (current_millis < prev_millis[i]) { // overflow
                if (!overflowIncreased) {
                    overFlowCounter++;
                    overflowIncreased = true;
                }
                prev_millis[i] = current_millis;
            }

            if (current_millis - prev_millis[i] >= delays[i]) {
                // update time
                prev_millis[i] = current_millis;
                DEBUG_PRINT("Run timer: ");
                DEBUG_PRINTLN(names[i]);
                (*callbacks[i])();
            }
        }
    }
}

// find the first available slot
// return -1 if none found
int AppTime::findFirstFreeSlot() {
    int i;

    // all slots are used
    if (numTimers >= MAX_TIMERS) {
        return -1;
    }

    // return the first slot with no callback (i.e. free)
    for (i = 0; i < MAX_TIMERS; i++) {
        if (callbacks[i] == 0) {
            return i;
        }
    }

    // no free slots found
    return -1;
}

int AppTime::setInterval(char *name, long d, timer_callback f) {
    int freeTimer;

    freeTimer = findFirstFreeSlot();
    if (freeTimer < 0) {
        return -1;
    }

    if (f == NULL) {
        return -1;
    }

    names[freeTimer] = name;
    delays[freeTimer] = d;
    callbacks[freeTimer] = f;
    prev_millis[freeTimer] = elapsed();

    numTimers++;

    return freeTimer;
}

int AppTime::getOverFlowCounter() {
    return overFlowCounter;
};

