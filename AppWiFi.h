#ifndef AppWiFi_h
#define AppWiFi_h

#include <Arduino.h>

class AppWiFi {
public:
    AppWiFi();

    ~AppWiFi();

    static void connect();

    static bool isConnected();
};

#endif /* AppWiFi_h */