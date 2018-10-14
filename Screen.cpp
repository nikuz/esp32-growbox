#include <Arduino.h>
#include <U8g2lib.h>
#include <RtcDS3231.h>

#include "Screen.h"
#include "AppTime.h"
#include "Tools.h"
#include "def.h"

static AppTime appTime;
static Tools tools;

#if PRODUCTION
U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 16, 17, U8X8_PIN_NONE);
#else
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 16, 17, U8X8_PIN_NONE);
#endif

Screen::Screen() {}
Screen::~Screen() {}

void Screen::initiate() {
    u8g2.begin();
    u8g2.enableUTF8Print();
    u8g2.clearDisplay();
    u8g2.setFont(u8g2_font_crox2cb_tr);
    
    const char loadingStr[] = "...";
    const u8g2_uint_t displayWidth = u8g2.getDisplayWidth();
    const u8g2_uint_t displayHeight = u8g2.getDisplayHeight();
    const int strWidth = u8g2.getStrWidth(loadingStr);

    u8g2.clearBuffer();
    u8g2.setCursor((displayWidth / 2) - (strWidth / 2), displayHeight / 2);
    u8g2.print(loadingStr);
    u8g2.sendBuffer();
}

void Screen::clearBuffer() {
    u8g2.clearBuffer();
}
void Screen::sendBuffer() {
    u8g2.sendBuffer();
}

void Screen::printTemperature(int temperature, int humidity) {
    if (isnan(temperature) || isnan(humidity)) {
        return;
    }
    const int line = 13;
    u8g2.setFont(u8g2_font_crox2cb_tr);

    // temperature
    const String temperatureStr = String(temperature);
    char temperatureCharStr[temperatureStr.length() + 1];
    strcpy(temperatureCharStr, temperatureStr.c_str());
    u8g2.setCursor(0, line);
    u8g2.print(temperatureStr);
    const int temperatureRow = u8g2.getStrWidth(temperatureCharStr);
    u8g2.drawCircle(temperatureRow + 5, line - 10, 2, U8G2_DRAW_ALL);

    // humidity
    u8g2.setCursor(temperatureRow + 11, line);
    u8g2.print(" | " + String(humidity) + "%");
}

void Screen::printDayStrip(int currentHour, int lightDayStart, int lightDayEnd) {
    const int line = 22;
    const u8g2_uint_t displayWidth = u8g2.getDisplayWidth();
    const u8g2_uint_t gapWidth = 1;
    const u8g2_uint_t boxWidth = (displayWidth / 24) - gapWidth;
    int i = 0;
    while (i < 24) {
        int boxHeight = i == currentHour ? 14 : 10;
        int margin = i == currentHour ? 2 : 0;
        u8g2_uint_t row = (i * boxWidth) + (i * gapWidth);
        if (tools.lightDayDiapasonMatch(i, lightDayStart, lightDayEnd)) {
            u8g2.drawBox(row, line - margin, boxWidth, boxHeight);
        } else {
            u8g2.drawFrame(row, line - margin, boxWidth, boxHeight);
        }
        i++;
    }
}

void Screen::printAppVersion() {
    u8g2_uint_t displayWidth = u8g2.getDisplayWidth();
    u8g2_uint_t displayHeight = u8g2.getDisplayHeight();
    u8g2.setFont(u8g2_font_u8glib_4_tf);
    const String versionStr = VERSION_MARKER + String(VERSION);
    char versionCharStr[versionStr.length() + 1];
    strcpy(versionCharStr, versionStr.c_str());
    u8g2_uint_t versioWidth = u8g2.getStrWidth(versionCharStr);
    u8g2.setCursor(displayWidth - versioWidth, displayHeight);
    u8g2.print(versionStr);
}

void Screen::printTime(struct tm localtime) {
    u8g2_uint_t displayHeight = u8g2.getDisplayHeight();

    u8g2.setFont(u8g2_font_6x10_tn);
    u8g2.setCursor(0, displayHeight);
    u8g2.print(appTime.getTimeString(localtime, "%02u/%02u/%04u %02u:%02u"));
}

void Screen::printHumidityWater(bool hasWater) {
    const int line = 47;
    u8g2.setFont(u8g2_font_crox2cb_tr);
    u8g2.setCursor(0, line);
    const char marker[] = "H";
    const int markerWidth = u8g2.getStrWidth(marker);
    const int radius = 5;
    const int left = markerWidth + radius + 2;
    const int top = line - radius;
    u8g2.print(marker);
    if (hasWater) {
        u8g2.drawDisc(left, top, radius, U8G2_DRAW_ALL);
    } else {
        u8g2.drawCircle(left, top, radius, U8G2_DRAW_ALL);
    }
}