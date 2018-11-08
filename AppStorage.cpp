#include <Arduino.h>
#include <Preferences.h>

#include "def.h"
#include "AppStorage.h"

static Preferences preferences;
static const char *TAG = "gbx_storage";

static StorageIntVariable intVariables[10];
static StorageStringVariable stringVariables[10];

AppStorage::AppStorage() {}

AppStorage::~AppStorage() {}

unsigned int AppStorage::getUInt(const char *key, int value) {
    unsigned int result = value;
    if (preferences.begin(TAG, false)) {
        result = preferences.getUInt(key, value);
    }
    preferences.end();

    return result;
}

String AppStorage::getString(const char *key, String value) {
    String result = value;
    if (preferences.begin(TAG, false)) {
        result = preferences.getString(key, value);
    }
    preferences.end();

    return result;
}

void AppStorage::putUInt(const char *key, int value) {
    if (preferences.begin(TAG, false)) {
        preferences.putUInt(key, value);
    }
    preferences.end();
}

void AppStorage::putString(const char *key, String value) {
    if (preferences.begin(TAG, false)) {
        preferences.putString(key, value);
    }
    preferences.end();
}

void AppStorage::setVariable(int *var, const char *key) {
    int urlCounts = *(&intVariables + 1) - intVariables;
    for (int i = 0; i < urlCounts; i++) {
        if (!intVariables[i].key) {
            intVariables[i] = StorageIntVariable(var, key);
            break;
        }
    }
}

void AppStorage::setVariable(String *var, const char *key) {
    int urlCounts = *(&stringVariables + 1) - stringVariables;
    for (int i = 0; i < urlCounts; i++) {
        if (!stringVariables[i].key) {
            stringVariables[i] = StorageStringVariable(var, key);
            break;
        }
    }
}

void AppStorage::restore() {
    if (preferences.begin(TAG, false)) {
        int intVarsLen = *(&intVariables + 1) - intVariables;
        for (int i = 0; i < intVarsLen; i++) {
            if (intVariables[i].key) {
                *intVariables[i].var = preferences.getUInt(intVariables[i].key, *intVariables[i].var);
            }
        }
        int stringVarsLen = *(&stringVariables + 1) - stringVariables;
        for (int i = 0; i < intVarsLen; i++) {
            if (stringVariables[i].key) {
                *stringVariables[i].var = preferences.getString(stringVariables[i].key, *stringVariables[i].var);
            }
        }
    }
    preferences.end();
}

