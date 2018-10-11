#define PRODUCTION 1    // production
// #define PRODUCTION 0    // development

#define countof(a) (sizeof(a) / sizeof(a[0]))

#define VERSION 33
#if PRODUCTION
#define VERSION_MARKER "P"
#else
#define VERSION_MARKER "D"
#endif

// dht
#define DHTTYPE DHT11
#define DHTPin  18

// relay
#define RELAY_1 32
#define RELAY_2 33