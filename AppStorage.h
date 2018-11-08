#ifndef AppStorage_h
#define AppStorage_h

#include <Arduino.h>

struct StorageIntVariable {
    int *var;
    const char *key;

    StorageIntVariable() {}

    StorageIntVariable(int *_var, const char *_key) : var(_var), key(_key) {}
};

struct StorageStringVariable {
    String *var;
    const char *key;

    StorageStringVariable() {}

    StorageStringVariable(String *_var, const char *_key) : var(_var), key(_key) {}
};

class AppStorage {
public:
    AppStorage();

    ~AppStorage();

    static unsigned int getUInt(const char *key, int value);

    static String getString(const char *key, String value);

    static void putUInt(const char *key, int value);

    static void putString(const char *key, String value);

    static void setVariable(int *var, const char *key);

    static void setVariable(String *var, const char *key);

    static void restore();
};

#endif /* AppStorage_h */