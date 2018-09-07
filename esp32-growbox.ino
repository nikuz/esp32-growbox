#include <DHT.h>
#include <EspOta.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <Ticker.h>
#include <Time.h>
#include <U8g2lib.h>
#include <WiFi.h>

WiFiClient client;
// Preferences preferences;

int VERSION = 10;
const char* TAG = "growbox";

// Your SSID and PSWD that the chip needs
// to connect to
const char* SSID = "bloom_down";
const char* PSWD = "Baijeep8pai5Ie";
IPAddress ip(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8);  // google DNS
IPAddress dns2(8, 8, 4, 4);  // google DNS

// S3 Bucket Config
// http://selfproduct.com/esp32-updates/growbox.bin
const String otaHost = "selfproduct.com";
const int otaPort = 80;  // Non https. For HTTPS 443. As of today, HTTPS doesn't work.
const String otaBin = "/esp32-updates/growbox.bin";

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

// ventilation settings
bool ventilationEnabled = false;
unsigned long ventilationEnableLastTime = millis();
unsigned long ventilationEnableTime;
const int ventilationCheckInterval = 60 * 10;  // check ventilation every 10 minutes
const int ventilationPeriodLength = 60;        // enable ventilation every 10 minutes
                                               // for 1 minute if humidity in range
const int ventilationHumidityMax = 70;
const int ventilationTemperatureMax = 35;
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
const String blynkPinVersion = "V5";
const String blynkPinPing = "V10";

EspOta otaUpdate(otaHost, otaPort, otaBin, TAG);
U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 16, 17, U8X8_PIN_NONE);

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
    } else {
        Serial.println(" Already connected");
    }
}

void printLocalTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

int getCurrentHour() {
    time_t now;
    time(&now);
    unsigned long localTime = now + (gmtOffset_sec * 2);
    unsigned long hours = (localTime % 86400L) / 3600;

    return (int)hours;
}

void otaUpdateHandler() { otaUpdate.begin(); }

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
        Serial.println("Ventilation ON.");
        digitalWrite(RELAY_2, LOW);
        ventilationEnabled = true;
        ventilationEnableTime = millis();
    }
}

void ventilationOff() {
    if (ventilationEnabled) {
        Serial.println("Ventilation OFF.");
        digitalWrite(RELAY_2, HIGH);
        ventilationEnabled = false;
        ventilationEnableLastTime = millis();
    }
}

void ventilationCheck() {
    if (millis() - (ventilationCheckInterval * 1000L) > ventilationEnableLastTime) {
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

String blynkPutPinUrl(String pinId, int value) { return blynkHost + "/update/" + pinId + "?value=" + String(value); }

void blynkGetData(int& localVariable, String pinId) {
    HTTPClient http;
    http.setTimeout(2000);
    http.addHeader("Connection", "close");

    const String pinUrl = blynkGetPinUrl(pinId);
    http.begin(pinUrl);
    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
        String response = http.getString();
        String value = response.substring(2, 4);
        int newValue = value.toInt();
        if (newValue != localVariable) {
            localVariable = value.toInt();
            screenRefresh();
        }
    } else {
        Serial.print("FAILED GET: " + String(httpResponseCode) + ": ");
        Serial.println(pinUrl);
    }
    http.end();
}

void blynkPostData() {
    HTTPClient http;
    http.setTimeout(2000);
    http.addHeader("Connection", "close");
    const String blynkSyncUrls[] = {
        blynkPutPinUrl(blynkPinTemperature, currentTemperature),
        blynkPutPinUrl(blynkPinHumidity, currentHumidity),
        blynkPutPinUrl(blynkPinLight, digitalRead(RELAY_1) == 1 ? 0 : 255),
        blynkPutPinUrl(blynkPinLightDayStart, lightDayStart),
        blynkPutPinUrl(blynkPinLightDayEnd, lightDayEnd),
        blynkPutPinUrl(blynkPinVentilation, digitalRead(RELAY_2) == 1 ? 0 : 255),
        blynkPutPinUrl(blynkPinVersion, VERSION),
        blynkPutPinUrl(blynkPinPing, 0),
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

    // Configure static IP and Google DNS
    WiFi.config(ip, gateway, subnet, dns1, dns2);
    // Connect to provided SSID and PSWD
    WiFi.begin(SSID, PSWD);
    wifiConnectEstablisher();

    // init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();

    wifiCheckConnectionTimer.attach(wifiCheckConnectionInterval, wifiConnectEstablisher);
    otaCheckUpdateTimer.attach(otaCheckUpdateInterval, otaUpdateHandler);
    ventilationCheckTimer.attach(ventilationCheckInterval, ventilationCheck);

    u8g2.begin();
    u8g2.enableUTF8Print();
    u8g2.clearDisplay();

    dht.begin();
}

void loop() {
    // temperature/humidity
    bool sameTemperature = true;
    bool sameHumidity = true;
    if (millis() - (DHTPReadDataInterval * 1000L) > DHTPReadDataLastTime) {
        float newHumidity = dht.readHumidity();
        float newTemperature = dht.readTemperature();
        if (isnan(newHumidity) || isnan(newTemperature)) {
            Serial.println("Failed to read from DHT sensor!");
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
    if (millis() - (hourCheckInterval * 1000L) > hourCheckLastTime) {
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

    if (millis() - (ventilationPeriodLength * 1000L) > ventilationEnableTime) {
        ventilationProphylaxis = false;
        ventilationOff();
    }

    if (millis() - (blynkSyncInterval * 1000L) > blynkSyncLastTime) {
        blynkGetData(lightDayStart, blynkPinLightDayStart);
        blynkGetData(lightDayEnd, blynkPinLightDayEnd);
        blynkGetData(pingNeedResponse, blynkPinPing);
        blynkPostData();
    }

    if (pingNeedResponse == 1) {
        pingNeedResponse = 0;
        blynkPingResponse();
    }
}
