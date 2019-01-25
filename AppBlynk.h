#ifndef AppBlynk_h
#define AppBlynk_h

#include <Arduino.h>

struct BlynkIntVariable {
    int *var;
    const char *pin;

    BlynkIntVariable() {}

    BlynkIntVariable(int *_var, const char *_pin) : var(_var), pin(_pin) {}
};

struct BlynkStringVariable {
    String *var;
    const char *pin;

    BlynkStringVariable() {}

    BlynkStringVariable(String *_var, const char *_pin) : var(_var), pin(_pin) {}
};

struct BlynkSyncVariable {
    const char *pin;
    bool synced;
};

class AppBlynk {
public:
    AppBlynk();

    ~AppBlynk();

    static void initiate();

    static void run();

    static void setVariable(int *var, const char *pin);

    static void setVariable(String *var, const char *pin);

    static void getData(int &localVariable, const char *pinId, int pinData, const bool storePreferences = true);

    static void getData(String &localVariable, const char *pinId, String pinData, const bool storePreferences = true);

    static void postData(const char *pinId, int value);

    static void postData(const char *pinId, String value);

    static void postDataNoCache(const char *pinId, int value);

    static void postDataNoCache(const char *pinId, String value);

    static int &getIntVariable(const char *pin);

    static String &getStringVariable(const char *pin);

    static void print(String value);

    static void print(char *value);

    static void print(int value);

    static void print(double value);

    static void println(String value);

    static void println(char *value);

    static void println(int value);

    static void println(double value);

    static void checkConnect();

    static void sync();

private:
    static int getPinById(const char *pinId);

    static int &getIntCacheValue(const char *pinId);

    static String &getStringCacheValue(const char *pinId);
};

#endif /* AppBlynk_h */
