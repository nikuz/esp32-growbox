#ifndef AppBlynk_h
#define AppBlynk_h

#include <Arduino.h>

struct BlynkIntVariable {
    int *var;
    const char *pin;
    bool store;

    BlynkIntVariable() {}

    BlynkIntVariable(int *_var, const char *_pin, bool _store = true) : var(_var), pin(_pin), store(_store) {}
};

struct BlynkStringVariable {
    String *var;
    const char *pin;
    bool store;

    BlynkStringVariable() {}

    BlynkStringVariable(String *_var, const char *_pin, bool _store = true) : var(_var), pin(_pin), store(_store) {}
};

class AppBlynk {
public:
    AppBlynk();

    ~AppBlynk();

    static void initiate();

    static void run();

    static void setVariable(int *var, const char *pin, bool store = true);

    static void setVariable(String *var, const char *pin, bool store = true);

    static void getData(int &localVariable, const char *pinId, int pinData, const bool storePreferences = true);

    static void getData(String &localVariable, const char *pinId, String pinData, const bool storePreferences = true);

    static void postData(String pinId, int value);

    static void postData(String pinId, String value);

    static int &getIntVariable(const char *pin);

    static String &getStringVariable(const char *pin);

    static void terminal(String value);

private:
    static int getPinById(String pinId);

    static int &getIntCacheValue(String pinId);

    static String &getStringCacheValue(String pinId);
};

#endif /* AppBlynk_h */
