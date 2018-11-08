#include <Arduino.h>
#include <Time.h>
#include <RtcDS3231.h>
#include <Wire.h>

#include "def.h"
#include "AppTime.h"
#include "AppWiFi.h"
#include "AppBlynk.h"

const char *ntpServer = "1.rs.pool.ntp.org";
const char *ntpServer2 = "0.europe.pool.ntp.org";
const char *ntpServer3 = "1.europe.pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 0;

RtcDS3231 <TwoWire> Rtc(Wire);
bool rtcBatteryIsLive = true;

AppTime::AppTime() {}

AppTime::~AppTime() {}

void AppTime::obtainSNTP() {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, ntpServer2, ntpServer3);
}

bool AppTime::localTime(struct tm *timeinfo) {
    return getLocalTime(timeinfo);
}

void AppTime::RTCBegin() {
    Rtc.Begin();
}

void AppTime::RTCUpdateByNtp() {
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    if (!Rtc.IsDateTimeValid()) {
        Rtc.SetDateTime(compiled);
        Serial.println("RTC lost confidence in the DateTime!");
    }
    if (!Rtc.GetIsRunning()) {
        Rtc.SetIsRunning(true);
        Serial.println("RTC was not actively running, starting now");
    }
    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) {
        Rtc.SetDateTime(compiled);
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
    }

    // Oct 10 2018
    // 00:06:21
    struct tm timeinfo = {0};
    if (AppTime::localTime(&timeinfo)) {
        static const char mon_name[][4] = {
                "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };
        static char ntpDate[15];
        sprintf(
                ntpDate,
                "%.3s%3d %d",
                mon_name[timeinfo.tm_mon],
                timeinfo.tm_mday,
                timeinfo.tm_year + 1900
        );
        static char ntpTime[15];
        sprintf(
                ntpTime,
                "%02u:%02u:%02u",
                timeinfo.tm_hour,
                timeinfo.tm_min,
                timeinfo.tm_sec
        );

        RtcDateTime ntpDateTime = RtcDateTime(ntpDate, ntpTime);
        Rtc.SetDateTime(ntpDateTime);
    }

    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
}

int AppTime::RTCGetTemperature() {
    RtcTemperature rtcTemp = Rtc.GetTemperature();
    return (int) rtcTemp.AsFloatDegC();
}

bool AppTime::RTCBattery() {
    return rtcBatteryIsLive;
}

bool AppTime::RTCIsDateTimeValid() {
    return Rtc.IsDateTimeValid();
}

struct tm AppTime::RTCGetCurrentTime() {
    RtcDateTime rtcTime = Rtc.GetDateTime();
    struct tm timeinfo = {
            tm_sec: rtcTime.Second(),
            tm_min: rtcTime.Minute(),
            tm_hour: rtcTime.Hour(),
            tm_mday: rtcTime.Day(),
            tm_mon: rtcTime.Month() - 1,      // 0-based
            tm_year: rtcTime.Year() - 1900,
            tm_wday: 0,
            tm_yday: 0,
            tm_isdst: 0
    };

    return timeinfo;
}

struct tm AppTime::getCurrentTime() {
    struct tm timeinfo = {0};
    if (!AppWiFi::isConnected() || !AppTime::localTime(&timeinfo)) {
        if (AppTime::RTCIsDateTimeValid()) {
            timeinfo = AppTime::RTCGetCurrentTime();
        } else {
            // AppTime::RTCBegin();
            // AppTime::RTCUpdateByNtp();
        }
    }
    if (!AppTime::RTCIsDateTimeValid()) {
        rtcBatteryIsLive = false;
    } else {
        rtcBatteryIsLive = true;
    }

    return timeinfo;
}

int AppTime::getCurrentHour() {
    struct tm currentTime = AppTime::getCurrentTime();
    return currentTime.tm_hour;
}

String AppTime::getTimeString(struct tm timeStruct, char format[]) {
    char timeString[20];
    snprintf_P(
            timeString,
            countof(timeString),
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
    AppBlynk::terminal("ntpTime: " + AppTime::getTimeString(AppTime::getCurrentTime()));
    AppBlynk::terminal("rtcTime: " + AppTime::getTimeString(AppTime::RTCGetCurrentTime()));
}

