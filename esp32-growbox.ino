#include <DHT.h>
#include <EspOta.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <Ticker.h>
#include <Time.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <RtcDS3231.h>
#include <Wire.h>
#include "def.h"
#include "Blynk.h"

WiFiClient client;
Preferences preferences;

int VERSION = 22;
const char* TAG = "growbox";

// Your SSID and PSWD that the chip needs
// to connect to
const char* SSID = "bloom";
const char* PSWD = "Baijeep8pai5Ie";
#if PRODUCTION
IPAddress ip(192, 168, 1, 100);
#else
IPAddress ip(192, 168, 1, 101);
#endif
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8);  // google DNS
IPAddress dns2(8, 8, 4, 4);  // google DNS

// OTA settings
String otaHost = "selfproduct.com";
const int otaPort = 80;  // Non https. For HTTPS 443. As of today, HTTPS doesn't work.
#if PRODUCTION
String otaBin = "/esp32-updates/growbox.bin";
#else
String otaBin = "/esp32-updates/growbox-dev.bin";
#endif

const char* ntpServer = "1.rs.pool.ntp.org";
const char* ntpServer2 = "0.europe.pool.ntp.org";
const char* ntpServer3 = "1.europe.pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

unsigned long wifiReconnectInterval = 1000L * 5L;
int wifiCheckConnectionInterval = 30;  // check Wifi connect every 30 seconds
Ticker wifiCheckConnectionTimer;

int otaCheckUpdateInterval = 60 * 2;  // check OTA update every 2 minutes
Ticker otaCheckUpdateTimer;

// temperature/humidity settings
#define DHTTYPE DHT11
const int DHTPin = 18;
DHT dht(DHTPin, DHTTYPE);
float currentTemperature;
float currentHumidity;
int DHTPReadDataInterval = 1;  // read sensor once in one second
unsigned long DHTPReadDataLastTime = millis();

// RtcDS3231
RtcDS3231<TwoWire> Rtc(Wire);
bool rtcBatteryIsLive = true;
int rtcTemperature;

// ventilation settings
bool ventilationEnabled = false;
unsigned long ventilationEnableLastTime = millis();
unsigned long ventilationEnableTime;
const int ventilationCheckInterval = 60 * 10;  // check ventilation every 10 minutes
const int ventilationPeriodLength = 60;        // enable ventilation every 10 minutes
                                               // for 1 minute if humidity in range
const int ventilationHumidityMax = 70;
int ventilationTemperatureMax = 35;
Ticker ventilationCheckTimer;
bool ventilationProphylaxis = false;

// light settings
unsigned int currentHour = 35;     // some
const int hourCheckInterval = 30;  // check current hour every 30 seconds
unsigned long hourCheckLastTime = millis();
int lightDayStart = 22;  // 22:00 by default
int lightDayEnd = 15;    // 15:00 by default
bool lightEnabled = false;

// relay
#define RELAY_1 32
#define RELAY_2 33

// uptime
String appUptime = "";

// Blynk settings
Blynk blynkClient;
const int blynkSyncInterval = 5;  // sync blynk state every 2 seconds
unsigned long blynkSyncLastTime = millis();
int pingNeedResponse = 0;

bool firstLoop = true;

EspOta otaUpdate(otaHost, otaPort, otaBin, TAG);
U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 16, 17, U8X8_PIN_NONE);

bool timerCheck(int interval, unsigned long lastInitiate) {
    unsigned long now = millis();
    return (interval * 1000L) < now && now - (interval * 1000L) > lastInitiate;
}

void calculateUptime() {
    char uptimeString[4];
    const float uptime = millis() / 1000.0L / 60.0L;
    dtostrf(uptime, 3, 1, uptimeString);
    appUptime = uptime > 60 ? String(uptime / 60) + "h" : String(uptimeString) + "m";
}

void getRTCTemperature() {
    RtcTemperature rtcTemp = Rtc.GetTemperature();
    rtcTemperature = (int)rtcTemp.AsFloatDegC();
}

void wifiConnectEstablisher() {
    // Wait for connection to establish
    Serial.print("Check WiFi connection...");
    unsigned long wifiReconnectTime = millis() + wifiReconnectInterval;
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("");
        Serial.println("Connecting to " + String(SSID));
        while (WiFi.status() != WL_CONNECTED) {
            Serial.print(".");  // Keep the serial monitor lit!
            delay(500);
            if (wifiReconnectTime < millis()) {
                Serial.println("Wifi need to reconnect");
                WiFi.reconnect();
                wifiReconnectTime = millis() + wifiReconnectInterval;
            }
        }

        // Connection Succeed
        Serial.println("");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("DNS address: ");
        Serial.println(WiFi.dnsIP());

        // init and get the time
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, ntpServer2, ntpServer3);
    } else {
        Serial.println(" Already connected");
    }
}

void RTCUpdateByNtp() {
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    if (!Rtc.IsDateTimeValid()) {
        blynkClient.terminal("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }
    if (!Rtc.GetIsRunning()) {
        blynkClient.terminal("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }
    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) {
        blynkClient.terminal("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        blynkClient.terminal("Failed to obtain time from NTP");
        return;
    }
    
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

    time_t ntpNow;
    time(&ntpNow);
    // unsigned long localTime = ntpNow + (gmtOffset_sec * 2);
    Rtc.SetDateTime(ntpNow);
}

unsigned long getCurrentTime() {
    RtcDateTime rtcTime = Rtc.GetDateTime();
    time_t ntpTime;
    time(&ntpTime);
    // unsigned long int ntpTime = now + (gmtOffset_sec * 2);
    //
    blynkClient.terminal("rtcTimeStamp: " + String(rtcTime));
    blynkClient.terminal("rtcTime: " + otaGetTimeStamp(rtcTime));
    blynkClient.terminal("ntpTime: " + otaGetTimeStamp(ntpTime));
    blynkClient.terminal("ntpTimeStamp: " + String(ntpTime));
    //
    if (ntpTime - rtcTime < 60) {
        Rtc.SetDateTime(ntpTime); // correct time
        rtcBatteryIsLive = true;
    } else if (ntpTime - rtcTime > 60) {
        blynkClient.terminal("RTC doesn't connect or battery down");
        // try to set RTC datetime
        Rtc.SetDateTime(ntpTime);
        rtcBatteryIsLive = false;
        return ntpTime;
    }
    return rtcTime;
}

int getCurrentHour() {
    time_t currentTime = getCurrentTime();
    struct tm* tmstruct = localtime(&currentTime);
    return tmstruct->tm_hour;
}

String dateCompensate(int value) {
    return value < 10 ? "0" + String(value) : String(value);
}

String otaGetTimeStamp(time_t currentTime) {
    struct tm* tmstruct = localtime(&currentTime);
    String year = String(tmstruct->tm_year + 1900);
    String month = dateCompensate(tmstruct->tm_mon + 1);
    String day = dateCompensate(tmstruct->tm_mday);
    String hour = dateCompensate(tmstruct->tm_hour);
    String minute = dateCompensate(tmstruct->tm_min);

    return year + "/" + month + "/" + day + " " + hour + ":" + minute;
}

void otaUpdateHandler() { 
    if (otaUpdate._host != otaHost || otaUpdate._bin != otaBin) {
        otaUpdate.updateEntries(otaHost, otaPort, otaBin);
    }
    // time_t currentTime = getCurrentTime() - (gmtOffset_sec * 2);
    otaUpdate.begin(otaGetTimeStamp(getCurrentTime()));
}

void screenPrintTemperature() {
    if (isnan(currentTemperature) || isnan(currentHumidity)) {
        return;
    }
    const int line = 13;
    u8g2.setFont(u8g2_font_crox2cb_tr);

    // temperature
    const String temperatureStr = String((int)currentTemperature);
    char temperatureCharStr[temperatureStr.length() + 1];
    strcpy(temperatureCharStr, temperatureStr.c_str());
    u8g2.setCursor(0, line);
    u8g2.print(temperatureStr);
    const int temperatureRow = u8g2.getStrWidth(temperatureCharStr);
    u8g2.drawCircle(temperatureRow + 5, line - 10, 2, U8G2_DRAW_ALL);

    // humidity
    u8g2.setCursor(temperatureRow + 11, line);
    u8g2.print(" | " + String((int)currentHumidity) + "%");
}

void screenPrintDayStrip() {
    const int line = 22;
    const u8g2_uint_t displayWidth = u8g2.getDisplayWidth();
    const u8g2_uint_t gapWidth = 1;
    const u8g2_uint_t boxWidth = (displayWidth / 24) - gapWidth;
    int i = 0;
    while (i < 24) {
        int boxHeight = i == currentHour ? 14 : 10;
        int margin = i == currentHour ? 2 : 0;
        u8g2_uint_t row = (i * boxWidth) + (i * gapWidth);
        if (lightDayDiapasonMatch(i)) {
            u8g2.drawBox(row, line - margin, boxWidth, boxHeight);
        } else {
            u8g2.drawFrame(row, line - margin, boxWidth, boxHeight);
        }
        i++;
    }
}

void screenPrintAppVersion() {
    u8g2_uint_t displayWidth = u8g2.getDisplayWidth();
    u8g2_uint_t displayHeight = u8g2.getDisplayHeight();
    u8g2.setFont(u8g2_font_u8glib_4_tf);
    const String versionStr = String((int)VERSION);
    char versionCharStr[versionStr.length() + 1];
    strcpy(versionCharStr, versionStr.c_str());
    u8g2_uint_t versioWidth = u8g2.getStrWidth(versionCharStr);
    u8g2.setCursor(displayWidth - versioWidth, displayHeight);
    u8g2.print(VERSION);
}

void screenRefresh() {
    u8g2.clearBuffer();
    screenPrintTemperature();
    screenPrintDayStrip();
    screenPrintAppVersion();
    u8g2.sendBuffer();
}

void ventilationOn() {
    if (!ventilationEnabled) {
        blynkClient.terminal("Ventilation ON.");
        digitalWrite(RELAY_2, LOW);
        ventilationEnabled = true;
        ventilationEnableTime = millis();
    }
}

void ventilationOff() {
    if (ventilationEnabled) {
        blynkClient.terminal("Ventilation OFF.");
        digitalWrite(RELAY_2, HIGH);
        ventilationEnabled = false;
        ventilationEnableLastTime = millis();
    }
}

void ventilationCheck() {
    if (millis() - (ventilationCheckInterval * 1000L) > ventilationEnableLastTime && !ventilationEnabled) {
        ventilationProphylaxis = true;
        ventilationOn();
    }
}

void lightOn() {
    if (!lightEnabled) {
        digitalWrite(RELAY_1, LOW);
        lightEnabled = true;
    }
}

void lightOff() {
    if (lightEnabled) {
        digitalWrite(RELAY_1, HIGH);
        lightEnabled = false;
    }
}

bool lightDayDiapasonMatch(int hour) {
    if (lightDayStart > lightDayEnd) {
        return hour >= lightDayStart || hour < lightDayEnd;
    }
    return hour >= lightDayStart && hour < lightDayEnd;
}

void blynkGetData(int& localVariable, const char* pinId, bool storePreferences = true) {
    const String pinData = blynkClient.getData(pinId);
    if (pinData == "") {
        return;
    }
    localVariable = pinData.toInt();
    if (storePreferences) {
        bool preferenceStarted = preferences.begin(TAG, false);
        if (preferenceStarted) {
            preferences.putUInt(pinId, pinData.toInt());
        }
        preferences.end();
    }
}

void blynkGetData(String& localVariable, const char* pinId, bool storePreferences = true) {
    const String pinData = blynkClient.getData(pinId);
    if (pinData == "") {
        return;
    }
    localVariable = pinData;
    if (storePreferences) {
        bool preferenceStarted = preferences.begin(TAG, false);
        if (preferenceStarted) {
            preferences.putString(pinId, pinData);
        }
        preferences.end();
    }
}

void setup() {
    // set relay off by default
    pinMode(RELAY_1, OUTPUT);
    digitalWrite(RELAY_1, HIGH);
    pinMode(RELAY_2, OUTPUT);
    digitalWrite(RELAY_2, HIGH);

    // Begin Serial
    Serial.begin(115200);
    while (!Serial) {
        ;  // wait for serial port to connect. Needed for native USB
    }

    // restore preferences
    bool preferenceStarted = preferences.begin(TAG, true);
    if (preferenceStarted) {
        lightDayStart = preferences.getUInt("lightDayStart", lightDayStart);
        lightDayEnd = preferences.getUInt("lightDayEnd", lightDayEnd);
        ventilationTemperatureMax = preferences.getUInt("ventilationTemperatureMax", ventilationTemperatureMax);
        otaHost = preferences.getString("otaHost", otaHost);
        otaBin = preferences.getString("otaBin", otaBin);
        preferences.end();
    }

    // Configure static IP and Google DNS
    WiFi.config(ip, gateway, subnet, dns1, dns2);
    // Connect to provided SSID and PSWD
    WiFi.begin(SSID, PSWD);
    wifiConnectEstablisher();

    wifiCheckConnectionTimer.attach(wifiCheckConnectionInterval, wifiConnectEstablisher);
    otaCheckUpdateTimer.attach(otaCheckUpdateInterval, otaUpdateHandler);
    ventilationCheckTimer.attach(ventilationCheckInterval, ventilationCheck);

    u8g2.begin();
    u8g2.enableUTF8Print();
    u8g2.clearDisplay();

    dht.begin();

    Rtc.Begin();
    RTCUpdateByNtp();
}

void loop() {
    // temperature/humidity
    bool sameTemperature = true;
    bool sameHumidity = true;
    if (timerCheck(DHTPReadDataInterval, DHTPReadDataLastTime) || firstLoop) {
        float newHumidity = dht.readHumidity();
        float newTemperature = dht.readTemperature();
        if (isnan(newHumidity) || isnan(newTemperature)) {
            blynkClient.terminal("Failed to read from DHT sensor!");
        } else {
            if (newTemperature > ventilationTemperatureMax || newHumidity > ventilationHumidityMax) {
                ventilationOn();
            } else if (!ventilationProphylaxis && (newTemperature <= ventilationTemperatureMax || newHumidity <= ventilationHumidityMax)) {
                ventilationOff();
            }
            if (newTemperature != currentTemperature) {
                currentTemperature = newTemperature;
                sameTemperature = false;
            }
            if (newHumidity != currentHumidity) {
                currentHumidity = newHumidity;
                sameHumidity = false;
            }
        }
        DHTPReadDataLastTime = millis();
    }

    // hour check
    bool sameHour = true;
    if (timerCheck(hourCheckInterval, hourCheckLastTime) || firstLoop) {
        int newHour = getCurrentHour();
        if (newHour != currentHour) {
            currentHour = newHour;
            sameHour = false;
        }
        if (lightDayDiapasonMatch(newHour)) {
            lightOn();
        } else {
            lightOff();
        }
        hourCheckLastTime = millis();
    }

    // screen
    if (!sameTemperature || !sameHumidity || !sameHour) {
        screenRefresh();
    }

    if ((timerCheck(ventilationPeriodLength, ventilationEnableTime) || firstLoop) && ventilationProphylaxis) {
        ventilationProphylaxis = false;
        ventilationOff();
    }

    if (timerCheck(blynkSyncInterval, blynkSyncLastTime)) {
        //
        calculateUptime();
        getRTCTemperature();
        //
        blynkGetData(lightDayStart, "lightDayStart");
        blynkGetData(lightDayEnd, "lightDayEnd");
        blynkGetData(ventilationTemperatureMax, "ventilationTemperatureMax");
        blynkGetData(otaHost, "otaHost");
        blynkGetData(otaBin, "otaBin");
        blynkGetData(pingNeedResponse, "ping", false);
        //
        #if PRODUCTION
        blynkClient.postData("temperature", currentTemperature);
        blynkClient.postData("humidity", currentHumidity);
        blynkClient.postData("light", digitalRead(RELAY_1) == 1 ? 0 : 255);
        blynkClient.postData("lightDayStart", lightDayStart);
        blynkClient.postData("lightDayEnd", lightDayEnd);
        blynkClient.postData("ventilation", digitalRead(RELAY_2) == 1 ? 0 : 255);
        blynkClient.postData("ventilationTemperatureMax", ventilationTemperatureMax);
        blynkClient.postData("version", VERSION);
        blynkClient.postData("ping", 0);
        blynkClient.postData("rtcBattery", rtcBatteryIsLive ? 255 : 0);
        blynkClient.postData("otaHost", otaHost);
        blynkClient.postData("otaBin", otaBin);
        blynkClient.postData("otaLastUpdateTime", otaUpdate.getUpdateTime());
        blynkClient.postData("uptime", appUptime);
        blynkClient.postData("rtcTemperature", rtcTemperature);
        #endif
        blynkSyncLastTime = millis();
    }

    if (pingNeedResponse == 1) {
        pingNeedResponse = 0;
        blynkClient.pingResponse();
    }

    if (firstLoop) {
        firstLoop = false;
    }
}
