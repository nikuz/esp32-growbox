#include <Arduino.h>

#include "def.h"
#include "Light.h"
#include "Tools.h"
#include "AppTime.h"
#include "AppSerial.h"

int lightIntensity;
unsigned long setIntensityInterval = 60L * 1000L; // set light intensity once in minute
unsigned long setIntensityLastTime = 0;

Light::Light() {}

Light::~Light() {}

// public

void Light::parseSerialCommand(const char *command, const char *param) {
    int value = Tools::StringToUint8(param);
    if (strcmp(command, "light") == 0) {
        lightIntensity = value;
    }
}

int Light::intensity() {
    return lightIntensity;
}

void Light::setIntensity(int &lightDayStart, int &lightDayEnd, int &lightMaxInt) {
    if (Tools::timerCheck(setIntensityInterval, setIntensityLastTime)) {
        const int currentHour = AppTime::getCurrentHour();
        const int currentMinute = AppTime::getCurrentMinute();
        int targetIntensity = 0;
        if (currentHour == lightDayStart) {
            targetIntensity = map(currentMinute, 0, 59, 0, lightMaxInt);
        } else if (currentHour == lightDayEnd - 1) {
            targetIntensity = map(currentMinute, 0, 59, lightMaxInt, 0);
        }
        char *targetIntensityChar = Tools::intToChar(targetIntensity);
        SerialFrame lightIntensityFrame = SerialFrame("knob", targetIntensityChar);
        AppSerial::sendFrame(&lightIntensityFrame);

        setIntensityLastTime = millis();
    }
}
