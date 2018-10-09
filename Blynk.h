#ifndef Blynk_h
#define Blynk_h

#include <Arduino.h>

class Blynk {
   public:
    Blynk();
    ~Blynk();

    void terminal(String value);
    void pingResponse();
    String getData(String pinId);
    void postData(String pinId, int value);
    void postData(String pinId, String value);
    
   private:
    String getPinId(String pinId);
    String getPinUrl(String pinId);
    String getPinUpdateUrl(String pinId);
    String putPinGetUrl(String pinId);
    String putPinUrl(String pinId, int value);
    String putPinUrl(String pinId, String value);
    void postPinData(String pinId, String data);
};

#endif
