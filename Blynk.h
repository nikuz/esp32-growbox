#ifndef Blynk_h
#define Blynk_h

#include <Arduino.h>

struct BlynkIntVariable {
    int* var;
    const char* pin;
    bool store;

    BlynkIntVariable() {}
    BlynkIntVariable(int* _var, const char* _pin, bool _store = true): var(_var), pin(_pin), store(_store) {}
};

struct BlynkStringVariable {
    String* var;
    const char* pin;
    bool store;

    BlynkStringVariable() {}
    BlynkStringVariable(String* _var, const char* _pin, bool _store = true): var(_var), pin(_pin), store(_store) {}
};

class Blynk {
   public:
    Blynk();
    ~Blynk();

    void terminal(String value);
    void notification(String message);
    void getData(int& localVariable, const char* pinId, bool storePreferences = true);
    void getData(String& localVariable, const char* pinId, bool storePreferences = true);
    void postData(String pinId, int value);
    void postData(String pinId, String value);

    void setVariable(int* var, const char* pin, bool store = true);
    void setVariable(String* var, const char* pin, bool store = true);
    void getProject();

   private:
    int getPinById(String pinId);
    char* getIdByPin(int pin);
    int& getIntCacheValue(String pinId);
    String& getStringCacheValue(String pinId);
    String getPinUrl(int pinId);
    String getPinUpdateUrl(int pinId);
    String putPinGetUrl(int pinId);
    String putPinUrl(int pinId, int value);
    String putPinUrl(int pinId, String value);
    String getPinData(String pinId);
    void postPinData(int pinId, String data);
};

#endif /* Blynk_h */
