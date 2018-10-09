#ifndef AppWiFi_h
#define AppWiFi_h

#include <Arduino.h>

class AppWiFi {
   public:
    AppWiFi();
    ~AppWiFi();

    void initiate();
    bool isConnected();
    void connect();

   private:
    void reConnect();
};

#endif /* AppWiFi_h */