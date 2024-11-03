#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "resources.h"

#define JSON_CONFIG_BUF_SIZE (16*1024)
#define JSON_FAVORITES_BUF_SIZE (64*64)
#define JSON_VERSION_INFO_BUF_SIZE (1024)
#define CONFIGURATION_FILENAME "/configuration.json"
#define FAVORITES_FILENAME "/favorites.json"


namespace configuration {

    String getResourceStr(const struct Resource& resource);
    DynamicJsonDocument getResourceJson(const struct Resource& resource, unsigned size=0);
    DynamicJsonDocument getDefautltConfiguration();
    DynamicJsonDocument getDefautltFavorites();
    DynamicJsonDocument getConfigSchema();
    String assertJson(JsonVariant configuration, String name);
    String assertConfiguration(JsonVariant configuration);
    DynamicJsonDocument getVersionInfo();
    String getConfigurationStr();
    DynamicJsonDocument getConfiguration();
    DynamicJsonDocument getFavorites();
    void resetConfiguration();
    void setConfiguration(DynamicJsonDocument configuration);
    void setFavorites(DynamicJsonDocument favorites);

}
