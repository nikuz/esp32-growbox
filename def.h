#define PRODUCTION 1    // production
//#define PRODUCTION 0    // development

#define VERSION 45

#if PRODUCTION
#define VERSION_MARKER "P"
#else
#define VERSION_MARKER "D"
#endif

// sensors
#define SOIL_SENSOR_1 1
#define SOIL_SENSOR_1_MIN 1024
#define SOIL_SENSOR_1_MAX 9

#define SOIL_SENSOR_2 2
#define SOIL_SENSOR_2_MIN 1024
#define SOIL_SENSOR_2_MAX 0

#define SOIL_SENSOR_3 3
#define SOIL_SENSOR_3_MIN 1024
#define SOIL_SENSOR_3_MAX 0

#define SOIL_SENSOR_4 4
#define SOIL_SENSOR_4_MIN 1024
#define SOIL_SENSOR_4_MAX 0