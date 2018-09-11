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

WiFiClient client;
Preferences preferences;

int VERSION = 17;
const char* TAG = "growbox";

// Your SSID and PSWD that the chip needs
// to connect to
const char* SSID = "bloom";
const char* PSWD = "Baijeep8pai5Ie";
IPAddress ip(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8);  // google DNS
IPAddress dns2(8, 8, 4, 4);  // google DNS

// OTA settings
String otaHost = "selfproduct.com";
const int otaPort = 80;  // Non https. For HTTPS 443. As of today, HTTPS doesn't work.
String otaBin = "/esp32-updates/growbox.bin";

const char* ntpServer = "pool.ntp.org";
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

// Blynk settings
const String blynkAuth = "a6124029c59b48988fd1e4f449787ab8";
const String blynkHost = "http://blynk-cloud.com/" + blynkAuth;
const int blynkSyncInterval = 5;  // sync blynk state every 2 seconds
unsigned long blynkSyncLastTime = millis();
int pingNeedResponse = 0;
const String blynkPinTemperature = "V0";
const String blynkPinHumidity = "V1";
const String blynkPinLight = "V2";
const String blynkPinLightDayStart = "V6";
const String blynkPinLightDayEnd = "V7";
const String blynkPinVentilation = "V3";
const String blynkPinVentilationTemperatureMax = "V8";
const String blynkPinVersion = "V5";
const String blynkPinPing = "V10";
const String blynkPinRtcBattery = "V9";
const String blynkPinOtaHost = "V20";
const String blynkPinOtaBin = "V21";
const String blynkPinTerminal = "V30";
const String blynkPinUptime = "V11";

EspOta otaUpdate(otaHost, otaPort, otaBin, TAG);
U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 16, 17, U8X8_PIN_NONE);

bool timerCheck(int interval, unsigned long lastInitiate) {
    return (interval * 1000L) < millis() && millis() - (interval * 1000L) > lastInitiate;
}

void terminalPrint(String value) {
    Serial.println(value);
    // HTTPClient http;
    // http.setTimeout(2000);
    // String requestUrl = blynGetPinUpdateUrl(blynkPinTerminal);
    // http.begin(requestUrl);
    // http.addHeader("Content-Type", "application/json");
    // int httpResponseCode = http.PUT("[\"\n" + value + "\"]");
    // if (httpResponseCode != 200) {
    //     Serial.print("FAILED TERMINAL PUT: " + String(httpResponseCode) + ": ");
    //     Serial.println(requestUrl);
    // }
    // http.end();
}

void wifiConnectEstablisher() {
    // Wait for connection to establish
    Serial.println("Check WiFi connection...");
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
    } else {
        Serial.println(" Already connected");
    }
}

void RTCUpdateByNtp() {
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    if (!Rtc.IsDateTimeValid()) {
        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }
    if (!Rtc.GetIsRunning()) {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }
    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time from NTP");
        return;
    }

    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

    time_t ntpNow;
    time(&ntpNow);
    unsigned long localTime = ntpNow + (gmtOffset_sec * 2);
    Rtc.SetDateTime(localTime);
}

int getCurrentHour() {
    RtcDateTime currTime = Rtc.GetDateTime();
    if (currTime.Year() < 2018) {
        terminalPrint("RTC doesn't connect or battery down");
        rtcBatteryIsLive = false;
        time_t now;
        time(&now);
        unsigned long localTime = now + (gmtOffset_sec * 2);
        unsigned long hours = (localTime % 86400L) / 3600;
        return (int)hours;
    }
    return currTime.Hour();
}

void otaUpdateHandler() { 
    if (otaUpdate._host != otaHost || otaUpdate._bin != otaBin) {
        otaUpdate.updateEntries(otaHost, otaPort, otaBin);
    }
    otaUpdate.begin();
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
        terminalPrint("Ventilation ON.");
        digitalWrite(RELAY_2, LOW);
        ventilationEnabled = true;
        ventilationEnableTime = millis();
    }
}

void ventilationOff() {
    if (ventilationEnabled) {
        terminalPrint("Ventilation OFF.");
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

void blynkPingResponse() {
    HTTPClient http;
    http.setTimeout(2000);
    http.begin(blynkHost + "/notify");
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST("{ \"body\": \"PONG\"}");
    http.end();
}

String blynkGetPinUrl(String pinId) { return blynkHost + "/get/" + pinId; }

String blynGetPinUpdateUrl(String pinId) { return blynkHost + "/update/" + pinId; }

String blynPutPinGetUrl(String pinId) { return blynkHost + "/update/" + pinId + "?value="; }

String blynkPutPinUrl(String pinId, int value) { return blynPutPinGetUrl(pinId) + String(value); }

String blynkPutPinUrl(String pinId, String value) { return blynPutPinGetUrl(pinId) + value; }

String blynkGetPinData(String pinId) {
    HTTPClient http;
    http.setTimeout(2000);
    http.addHeader("Connection", "close");

    const String pinUrl = blynkGetPinUrl(pinId);
    String response = "";
    http.begin(pinUrl);
    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
        response = http.getString();
        response.replace("[\"", "");
        response.replace("\"]", "");
    } else {
        Serial.print("FAILED GET: " + String(httpResponseCode) + ": ");
        Serial.println(pinUrl);
    }
    http.end();

    return response;
}

void blynkGetData(int& localVariable, String pinId, const char* preferenceItem = "none") {
    const String pinData = blynkGetPinData(pinId);
    if (pinData == "") {
        return;
    }
    localVariable = pinData.toInt();
    if (preferenceItem != "none") {
        bool preferenceStarted = preferences.begin(TAG, false);
        if (preferenceStarted) {
            preferences.putUInt(preferenceItem, pinData.toInt());
        }
        preferences.end();
    }
}

void blynkGetData(String& localVariable, String pinId, const char* preferenceItem = "none") {
    const String pinData = blynkGetPinData(pinId);
    if (pinData == "") {
        return;
    }
    localVariable = pinData;
    if (preferenceItem != "none") {
        bool preferenceStarted = preferences.begin(TAG, false);
        if (preferenceStarted) {
            preferences.putString(preferenceItem, pinData);
        }
        preferences.end();
    }
}

void blynkPostData() {
    HTTPClient http;
    http.setTimeout(2000);
    http.addHeader("Connection", "close");

    const unsigned int uptime = millis() / 1000L / 60L;
    const String blynkSyncUrls[] = {
        blynkPutPinUrl(blynkPinTemperature, currentTemperature),
        blynkPutPinUrl(blynkPinHumidity, currentHumidity),
        blynkPutPinUrl(blynkPinLight, digitalRead(RELAY_1) == 1 ? 0 : 255),
        blynkPutPinUrl(blynkPinLightDayStart, lightDayStart),
        blynkPutPinUrl(blynkPinLightDayEnd, lightDayEnd),
        blynkPutPinUrl(blynkPinVentilation, digitalRead(RELAY_2) == 1 ? 0 : 255),
        blynkPutPinUrl(blynkPinVentilationTemperatureMax, ventilationTemperatureMax),
        blynkPutPinUrl(blynkPinVersion, VERSION),
        blynkPutPinUrl(blynkPinPing, 0),
        blynkPutPinUrl(blynkPinRtcBattery, rtcBatteryIsLive ? 255 : 0),
        blynkPutPinUrl(blynkPinOtaHost, otaHost),
        blynkPutPinUrl(blynkPinOtaBin, otaBin),
        blynkPutPinUrl(blynkPinUptime, uptime > 60 ? String(uptime / 60) + "h" : String(uptime) + "m"),
    };

    int urlCounts = *(&blynkSyncUrls + 1) - blynkSyncUrls;
    for (int i = 0; i < urlCounts; i++) {
        http.begin(blynkSyncUrls[i]);
        int httpResponseCode = http.GET();
        if (httpResponseCode != 200) {
            Serial.print("FAILED POST: " + String(httpResponseCode) + ": ");
            Serial.println(blynkSyncUrls[i]);
        }
    }
    http.end();
    blynkSyncLastTime = millis();
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

    // init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

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
    if (timerCheck(DHTPReadDataInterval, DHTPReadDataLastTime)) {
        float newHumidity = dht.readHumidity();
        float newTemperature = dht.readTemperature();
        if (isnan(newHumidity) || isnan(newTemperature)) {
            terminalPrint("Failed to read from DHT sensor!");
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
    if (timerCheck(hourCheckInterval, hourCheckLastTime)) {
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

    if (timerCheck(ventilationPeriodLength, ventilationEnableTime) && ventilationProphylaxis) {
        ventilationProphylaxis = false;
        ventilationOff();
    }

    if (millis() - (blynkSyncInterval * 1000L) > blynkSyncLastTime) {
        blynkGetData(lightDayStart, blynkPinLightDayStart, "lightDayStart");
        blynkGetData(lightDayEnd, blynkPinLightDayEnd, "lightDayEnd");
        blynkGetData(ventilationTemperatureMax, blynkPinVentilationTemperatureMax, "ventilationTemperatureMax");
        blynkGetData(otaHost, blynkPinOtaHost, "otaHost");
        blynkGetData(otaBin, blynkPinOtaBin, "otaBin");
        blynkGetData(pingNeedResponse, blynkPinPing);
        blynkPostData();
    }

    if (pingNeedResponse == 1) {
        pingNeedResponse = 0;
        blynkPingResponse();
    }
}
