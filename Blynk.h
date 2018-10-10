#ifndef Blynk_h
#define Blynk_h

#include <Arduino.h>

class Blynk {
   public:
    Blynk();
    ~Blynk();

    void terminal(String value);
    void pingResponse();
    void getData(int& localVariable, const char* pinId, bool storePreferences = true);
    void getData(String& localVariable, const char* pinId, bool storePreferences = true);
    void postData(String pinId, int value);
    void postData(String pinId, String value);

   private:
    String getPinId(String pinId);
    int& getIntCacheValue(String pinId);
    String& getStringCacheValue(String pinId);
    String getPinUrl(String pinId);
    String getPinUpdateUrl(String pinId);
    String putPinGetUrl(String pinId);
    String putPinUrl(String pinId, int value);
    String putPinUrl(String pinId, String value);
    String getPinData(String pinId);
    void postPinData(String pinId, String data);
};

#endif
