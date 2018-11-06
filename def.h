#define PRODUCTION 1    // production
// #define PRODUCTION 0    // development

#define countof(a) (sizeof(a) / sizeof(a[0]))

#define VERSION 41

#if PRODUCTION
#define VERSION_MARKER "P"
#else
#define VERSION_MARKER "D"
#endif

// sensors
#define DHTTYPE DHT22
#define DHTPin  18
#define HUMIDITY_WATER_SENSOR 13

#define SOIL_SENSOR_1 39
#define SOIL_SENSOR_1_MIN 3410
#define SOIL_SENSOR_1_MAX 1350

#define SOIL_SENSOR_2 34
#define SOIL_SENSOR_2_MIN 3200
#define SOIL_SENSOR_2_MAX 1580

#define SOIL_SENSOR_3 36
#define SOIL_SENSOR_3_MIN 3405
#define SOIL_SENSOR_3_MAX 1355

#define SOIL_SENSOR_4 35
#define SOIL_SENSOR_4_MIN 3225
#define SOIL_SENSOR_4_MAX 1320

// relay
#define RELAY_1 32 // lamp
#define RELAY_2 33 // ventilation
#define RELAY_3 26 // humidity