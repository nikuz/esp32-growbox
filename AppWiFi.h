#ifndef AppWiFi_h
#define AppWiFi_h

#include <Arduino.h>

class AppWiFi {
public:
    AppWiFi();

    ~AppWiFi();

    static void initiate();

    static const char *getSSID();

    static const char *getPSWD();

    static bool isConnected();

    static void connect();

private:
    static void reConnect();
};

#endif /* AppWiFi_h */