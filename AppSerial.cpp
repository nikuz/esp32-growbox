#include <Arduino.h>

#include "AppSerial.h"

static char command[10];
static char param[20];
static char chunk;
static byte i = 0;
static boolean delimiterPassed = false;
static boolean gotCommand = false;

AppSerial::AppSerial() {}

AppSerial::~AppSerial() {}

//unsigned long bytesCounter = 0;
//unsigned long testTime = 0;

SerialFrame AppSerial::getFrame() {
    delimiterPassed = false;
    gotCommand = false;
    i = 0;

    if (Serial2.available() > 0) {
        memset(command, 0, sizeof(command));
        memset(param, 0, sizeof(param));
    }

    while (Serial2.available() > 0 && gotCommand == false) {
        chunk = Serial2.read();
//        //
//        bytesCounter++;
//        //
        if (chunk != frameDelimiter && chunk != frameEndMarker) {
            if (delimiterPassed) {
                param[i] = chunk;
            } else {
                command[i] = chunk;
            }
            i++;
        } else if (chunk == frameDelimiter) {
            command[i] = '\0'; // terminate the command
            i = 0;
            delimiterPassed = true;
        } else if (chunk == frameEndMarker) {
            if (delimiterPassed) {
                param[i] = '\0'; // terminate the param
            }
            gotCommand = true;
            i = 0;
        }
    }

    if (gotCommand) {
        Serial.print(command);
        Serial.print(frameDelimiter);
        Serial.println(param);

//        //
//        if (strcmp(param, "first") == 0) {
//            testTime = millis();
//        }
//        if (strcmp(param, "last") == 0) {
//            Serial.print("Bytes: ");
//            Serial.println(bytesCounter);
//            Serial.print("Time: ");
//            Serial.println(millis() - testTime);
//        }
//        //

        return SerialFrame(command, param);
    }

    return SerialFrame("", "");
}

void AppSerial::sendFrame(SerialFrame *serialFrame) {
    Serial2.write(serialFrame->command);
    Serial2.write(serialFrame->delimiter);
    Serial2.write(serialFrame->param);
    Serial2.write(serialFrame->endMarker);
    Serial2.flush();
}
