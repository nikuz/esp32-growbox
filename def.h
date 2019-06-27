#define PRODUCTION 1    // production
//#define PRODUCTION 0    // development
//#define DEBUG 1

#define VERSION_ID "51"

#if PRODUCTION
#define VERSION_MARKER "P"
#else
#define VERSION_MARKER "D"
#endif

#define VERSION VERSION_MARKER VERSION_ID

#ifdef DEBUG
#define DEBUG_PRINT(x)    Serial.print(x)
#define DEBUG_PRINTLN(x)  Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

// sensors
#define SOIL_SENSOR_1 1
#define SOIL_SENSOR_1_MIN 556
#define SOIL_SENSOR_1_MAX 291

#define SOIL_SENSOR_2 2
#define SOIL_SENSOR_2_MIN 539
#define SOIL_SENSOR_2_MAX 250

#define SOIL_SENSOR_3 3
#define SOIL_SENSOR_3_MIN 550
#define SOIL_SENSOR_3_MAX 316

#define SOIL_SENSOR_4 4
#define SOIL_SENSOR_4_MIN 556
#define SOIL_SENSOR_4_MAX 314
