#ifndef Tools_h
#define Tools_h

#include <Arduino.h>

class Tools {
public:
    Tools();

    ~Tools();

    static bool lightDayDiapasonMatch(int hour, int lightDayStart, int lightDayEnd);

    static String getUptime();

    static String getHeaderValue(String header, String headerName);

    static uint8_t StringToUint8(const char *pString);

    static char *getCharArray(char *args[], int len);

    static char *intToChar(unsigned int value);

    static char *stringReplace(char* str, char *find, char *replace);

    static bool timerCheck(int interval, unsigned long lastInitiate);
};

#endif /* Tools_h */