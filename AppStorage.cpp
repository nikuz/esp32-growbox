#include <Arduino.h>
#include <Preferences.h>

#include "def.h"
#include "AppStorage.h"

static Preferences preferences;
static const char* TAG = "gbx_storage";

AppStorage::AppStorage() {}
AppStorage::~AppStorage() {}

unsigned int AppStorage::getUInt(const char* key, int value) {
    unsigned int result = value;
    if (preferences.begin(TAG, false)) {
        result = preferences.getUInt(key, value);
    }
    preferences.end();

    return result;
}

String AppStorage::getString(const char* key, String value) {
    String result = value;
    if (preferences.begin(TAG, false)) {
        result = preferences.getString(key, value);
    }
    preferences.end();

    return result;
}

void AppStorage::putUInt(const char* key, int value) {
    if (preferences.begin(TAG, false)) {
        preferences.putUInt(key, value);
    }
    preferences.end();
}

void AppStorage::putString(const char* key, String value) {
    if (preferences.begin(TAG, false)) {
        preferences.putString(key, value);
    }
    preferences.end();
}



