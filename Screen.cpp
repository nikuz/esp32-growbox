#include <Arduino.h>
#include <U8g2lib.h>

#include "def.h"
#include "Screen.h"
#include "AppTime.h"
#include "Tools.h"
#include "Sensor.h"

#if PRODUCTION
U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 21, 22, U8X8_PIN_NONE);
#else
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 21, 22, U8X8_PIN_NONE);
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
//    u8g2.drawFrame(0, 0, u8g2.getDisplayWidth(), u8g2.getDisplayHeight());
    u8g2.sendBuffer();
}

void Screen::clearBuffer() {
    u8g2.clearBuffer();
}

void Screen::sendBuffer() {
    u8g2.sendBuffer();
}

void Screen::printTemperature(float temperature, float humidity) {
    if (isnan(temperature) || isnan(humidity)) {
        return;
    }
    const int line = 13;
    u8g2.setFont(u8g2_font_crox2cb_tr);

    // temperature
    char temperatureStr[3];
    dtostrf(temperature, 2, 0, temperatureStr);
    u8g2.setCursor(0, line);
    u8g2.print(temperatureStr);
    const int temperatureRow = u8g2.getStrWidth(temperatureStr);
    u8g2.drawCircle(temperatureRow + 5, line - 10, 2, U8G2_DRAW_ALL);

    // humidity
    char humidityStr[3];
    dtostrf(humidity, 2, 0, humidityStr);
    const char *separator = " | ";
    u8g2.drawStr(temperatureRow + 11, line, separator);
    const int separatorRow = u8g2.getStrWidth(separator);
    u8g2.drawStr(temperatureRow + 11 + separatorRow, line, humidityStr);
    const int humidityRow = u8g2.getStrWidth(humidityStr);
    u8g2.drawStr(temperatureRow + 11 + separatorRow + humidityRow, line, "%");
}

void Screen::printDayStrip(int currentHour, int lightDayStart, int lightDayEnd) {
    if (currentHour == -1) {
        return;
    }
    const int line = 22;
    const u8g2_uint_t displayWidth = u8g2.getDisplayWidth();
    const u8g2_uint_t gapWidth = 1;
    const u8g2_uint_t boxWidth = (displayWidth / 24) - gapWidth;
    int i = 0;
    while (i < 24) {
        int boxHeight = i == currentHour ? 14 : 10;
        int margin = i == currentHour ? 2 : 0;
        u8g2_uint_t row = (i * boxWidth) + (i * gapWidth);
        if (AppTime::lightDayDiapasonMatch(i)) {
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
    u8g2.setCursor(displayWidth - 15, displayHeight);
    u8g2.print(VERSION);
}

void Screen::printTime(struct tm localtime) {
    u8g2_uint_t displayHeight = u8g2.getDisplayHeight();

    u8g2.setFont(u8g2_font_6x10_tn);
    u8g2.setCursor(0, displayHeight);
    u8g2.print(AppTime::getTimeString(localtime, "%02u/%02u/%04u %02u:%02u"));
}

void Screen::printUptime() {
    u8g2_uint_t displayHeight = u8g2.getDisplayHeight();

    u8g2.setFont(u8g2_font_crox2cb_tr);
    u8g2.setCursor(0, displayHeight);
    u8g2.print(Tools::getUptime());
}

void printMarkerWithCircle(char *marker, bool enabled, int line, int margin) {
    u8g2.setFont(u8g2_font_crox2cb_tr);
    u8g2.setCursor(margin, line);
    const int markerWidth = u8g2.getStrWidth(marker);
    const int radius = 5;
    const int left = markerWidth + radius + 2 + margin;
    const int top = line - radius;
    u8g2.print(marker);
    if (enabled) {
        u8g2.drawDisc(left, top, radius, U8G2_DRAW_ALL);
    } else {
        u8g2.drawCircle(left, top, radius, U8G2_DRAW_ALL);
    }
}

void Screen::printHumidityWater(bool hasWater) {
    printMarkerWithCircle("H", hasWater, 47, 0);
}

void Screen::printWater(bool hasWater) {
    printMarkerWithCircle("W", hasWater, 47, 35);
}

void Screen::printWaterLeakage(bool leakageDetected) {
    printMarkerWithCircle("L", leakageDetected, 47, 70);
}

void Screen::refresh() {
    const int currentHour = AppTime::getCurrentHour();
    int &lightDayStart = AppTime::getLightDayStart();
    int &lightDayEnd = AppTime::getLightDayEnd();

    clearBuffer();
    printTemperature(Sensor::temperatureGet(), Sensor::humidityGet());
    printHumidityWater(Sensor::humidityHasWater());
    printWater(Sensor::wateringHasWater());
    printWaterLeakage(Sensor::waterLeakageDetected());
    printAppVersion();

    if (currentHour != -1) {
        printDayStrip(currentHour, lightDayStart, lightDayEnd);
//        Screen::printTime(AppTime::getCurrentTime());
    }
    printUptime();

    sendBuffer();
}
