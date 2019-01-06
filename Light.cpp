#include <Arduino.h>

#include "def.h"
#include "Light.h"
#include "Tools.h"
#include "AppTime.h"
#include "AppSerial.h"

static LightVariable variables[2];
static int blankVariable = -1;

int lightIntensity;

Light::Light() {}

Light::~Light() {}

int &Light::getVariable(const char *key) {
    const int varsLen = *(&variables + 1) - variables;
    for (int i = 0; i < varsLen; i++) {
        if (variables[i].key == key) {
            return *variables[i].var;
        }
    }

    return blankVariable;
}

// public

void Light::setVariable(int *var, const char *key) {
    int varsLen = *(&variables + 1) - variables;
    for (int i = 0; i < varsLen; i++) {
        if (!variables[i].key) {
            variables[i] = LightVariable(var, key);
            break;
        }
    }
}

void Light::parseSerialCommand(const char *command, const char *param) {
    int value = atoi(param);
    if (strcmp(command, "light") == 0) {
        lightIntensity = value;
    }
}

int Light::intensity() {
    return lightIntensity;
}

void Light::setIntensity() {
    int &lightMaxInt = getVariable("lightMaxInt");
    const int currentHour = AppTime::getCurrentHour();
    const int currentMinute = AppTime::getCurrentMinute();
    int& lightDayStart = AppTime::getLightDayStart();
    int& lightDayEnd = AppTime::getLightDayEnd();
    int targetIntensity = 0;
    if (currentHour == lightDayStart) {
        targetIntensity = map(currentMinute, 0, 59, 0, lightMaxInt);
    } else if (currentHour == lightDayEnd - 1) {
        targetIntensity = map(currentMinute, 0, 59, lightMaxInt, 0);
    } else if (AppTime::lightDayDiapasonMatch(currentHour)) {
        targetIntensity = lightMaxInt;
    }
    char *targetIntensityChar = Tools::intToChar(targetIntensity);
    SerialFrame lightIntensityFrame = SerialFrame("knob", targetIntensityChar);
    AppSerial::sendFrame(&lightIntensityFrame);
}
