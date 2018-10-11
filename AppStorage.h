#ifndef AppStorage_h
#define AppStorage_h

#include <Arduino.h>

struct StorageIntVariable {
    int* var;
    const char* key;

    StorageIntVariable() {}
    StorageIntVariable(int* _var, const char* _key): var(_var), key(_key) {}
};

struct StorageStringVariable {
    String* var;
    const char* key;

    StorageStringVariable() {}
    StorageStringVariable(String* _var, const char* _key): var(_var), key(_key) {}
};

class AppStorage {
   public:
    AppStorage();
    ~AppStorage();

    unsigned int getUInt(const char* key, int value);
    String getString(const char* key, String value);
    void putUInt(const char* key, int value);
    void putString(const char* key, String value);

    void setVariable(int* var, const char* key);
    void setVariable(String* var, const char* key);
    void restore();
};

#endif /* AppStorage_h */