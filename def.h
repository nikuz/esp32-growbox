#define PRODUCTION 1    // production
// #define PRODUCTION 0    // development

#define countof(a) (sizeof(a) / sizeof(a[0]))

#define VERSION 38

#if PRODUCTION
#define VERSION_MARKER "P"
#else
#define VERSION_MARKER "D"
#endif

// sensors
#define DHTTYPE DHT11
#define DHTPin  18
#define HUMIDITY_WATER_SENSOR 13
#define SOIL_SENSOR_1 34
#define SOIL_SENSOR_1_MIN 3200
#define SOIL_SENSOR_1_MAX 1580

#define SOIL_SENSOR_2 35
#define SOIL_SENSOR_2_MIN 3225
#define SOIL_SENSOR_2_MAX 1320

// relay
#define RELAY_1 32 // lamp
#define RELAY_2 33 // ventilation
#define RELAY_3 26 // humidity