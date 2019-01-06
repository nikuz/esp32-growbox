#include <Arduino.h>

#include "Tools.h"

Tools::Tools() {}

Tools::~Tools() {}

String Tools::getUptime() {
    char uptimeString[4];
    const float uptime = millis() / 1000.0L / 60.0L;
    dtostrf(uptime, 3, 1, uptimeString);
    return uptime > 60 ? String(uptime / 60) + "h" : String(uptimeString) + "m";
}

// Utility to extract header value from headers
String Tools::getHeaderValue(String header, String headerName) {
    return header.substring(strlen(headerName.c_str()));
}

uint8_t Tools::StringToUint8(const char *pString) {
    uint8_t value = 0;

    // skip leading 0 and spaces
    while ('0' == *pString || *pString == ' ') {
        pString++;
    }

    // calculate number until we hit non-numeral char
    while ('0' <= *pString && *pString <= '9') {
        value *= 10;
        value += *pString - '0';
        pString++;
    }
    return value;
}

char *Tools::getCharArray(char *args[], int len) {
    static char result[50];
    strcpy(result, "");

    for (int i = 0; i < len; i++) {
        strcat(result, args[i]);
    }

    return result;
}

char *Tools::intToChar(unsigned int value) {
    static char result[5];
    int strLength = 1;
    if (value > 9) {
        strLength = 2;
    } else if (value > 99) {
        strLength = 3;
    } else if (value > 999) {
        strLength = 4;
    } else if (value > 9999) {
        strLength = 5;
    }
    dtostrf(value, strLength, 0, result);

    return result;
}

char *Tools::stringReplace(char *str, char *find, char *replace) {
    int len  = strlen(str);
    int lena = strlen(find);
    int lenb = strlen(replace);

    for (char* p = str; p = strstr(p, find); ++p) {
        if (lena != lenb) { // shift end as needed
            memmove(p + lenb, p + lena, len - (p - str) + lenb);
        }

        memcpy(p, replace, lenb);
    }
    return str;
}

