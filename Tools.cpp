#include <Arduino.h>

#include "Tools.h"

Tools::Tools() {}
Tools::~Tools() {}

bool Tools::lightDayDiapasonMatch(int hour, int lightDayStart, int lightDayEnd) {
    if (lightDayStart > lightDayEnd) {
        return hour >= lightDayStart || hour < lightDayEnd;
    }
    return hour >= lightDayStart && hour < lightDayEnd;
}

String Tools::getUptime() {
    char uptimeString[4];
    const float uptime = millis() / 1000.0L / 60.0L;
    dtostrf(uptime, 3, 1, uptimeString);
    return uptime > 60 ? String(uptime / 60) + "h" : String(uptimeString) + "m";
}
