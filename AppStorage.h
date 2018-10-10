#ifndef AppStorage_h
#define AppStorage_h

#include <Arduino.h>

class AppStorage {
   public:
    AppStorage();
    ~AppStorage();

    unsigned int getUInt(const char* key, int value);
    String getString(const char* key, String value);
    void putUInt(const char* key, int value);
    void putString(const char* key, String value);
};

#endif /* AppStorage_h */