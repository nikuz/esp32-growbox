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

static const char mon_name[][4] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

AppTime::AppTime() {}

AppTime::~AppTime() {}

void AppTime::obtainSNTP() {
    if (AppWiFi::isConnected()) {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, ntpServer2, ntpServer3);
    }
}

void AppTime::parseSerialCommand(const char *command, const char *param) {
    // 10/10/2018 00:09:21
    if (strcmp(command, "time") == 0) {
        uint8_t month = -1;
        uint8_t day = -1;
        uint8_t year = -1;
        uint8_t hours = -1;
        uint8_t minutes = -1;
        uint8_t seconds = -1;

        char *string, *found;
        string = strdup(param);

        int i = 0;
        while((found = strsep(&string, " ")) != NULL) {
            char *substring, *subfound;
            substring = strdup(found);
            if (i == 0) {
                int dateI = 0;
                while((subfound = strsep(&substring, "/")) != NULL) {
                    int value = Tools::StringToUint8(subfound);
                    switch (dateI) {
                        case 0:
                            month = value;
                            break;
                        case 1:
                            day = value;
                            break;
                        case 2:
                            year = Tools::StringToUint8(subfound + 2);
                            break;
                    }
                    dateI++;
                }
            } else {
                int timeI = 0;
                while((subfound = strsep(&substring, ":")) != NULL) {
                    int value = Tools::StringToUint8(subfound);
                    switch (timeI) {
                        case 0:
                            hours = value;
                            break;
                        case 1:
                            minutes = value;
                            break;
                        case 2:
                            seconds = value;
                            break;
                    }
                    timeI++;
                }
            }
            free(substring);
            i++;
        }
        free(string);

        RTCCurrentTime = {
                tm_sec: seconds,
                tm_min: minutes,
                tm_hour: hours,
                tm_mday: day,
                tm_mon: month - 1, // 0 based
                tm_year: 2000 + year - 1900,
                tm_wday: 0,
                tm_yday: 0,
                tm_isdst: 0
        };
    }
    if (strcmp(command, "ttemp") == 0) {
        int value = Tools::StringToUint8(param);
        rtcTemperature = value;
    }
    if (strcmp(command, "tbtr") == 0) {
        int value = Tools::StringToUint8(param);
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

struct tm AppTime::RTCGetCurrentTime() {
    return RTCCurrentTime;
}

struct tm AppTime::getCurrentTime() {
    struct tm timeinfo = {0};
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

void AppTime::print() {
    struct tm timeinfo = {0};
    AppTime::localTime(&timeinfo);

    char *ntpTime[] = {"ntpTime: ", AppTime::getTimeString(timeinfo)};
    char *ntpTimeStr = Tools::getCharArray(ntpTime, 2);
    AppBlynk::terminal(ntpTimeStr);

    char *rtcTime[] = {"rtcTime: ", AppTime::getTimeString(RTCCurrentTime)};
    char *rtcTimeStr = Tools::getCharArray(rtcTime, 2);
    AppBlynk::terminal(rtcTimeStr);
}

