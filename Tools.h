#ifndef Tools_h
#define Tools_h

#include <Arduino.h>

class Tools {
   public:
    Tools();
    ~Tools();

    bool lightDayDiapasonMatch(int hour, int lightDayStart, int lightDayEnd);
    String getUptime();
    String getHeaderValue(String header, String headerName);
};

#endif /* Tools_h */