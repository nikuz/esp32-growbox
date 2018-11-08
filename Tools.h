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
};

#endif /* Tools_h */