#ifndef Tools_h
#define Tools_h

#include <Arduino.h>

class Tools {
public:
    Tools();

    ~Tools();

    static String getUptime();

    static String getHeaderValue(String header, String headerName);

    static uint8_t StringToUint8(const char *pString);

    static char *getCharArray(char *args[], int len);

    static char *intToChar(unsigned int value);

    static char *stringReplace(char* str, char *find, char *replace);
};

#endif /* Tools_h */