#include "configuration.h"

#include <SPIFFS.h>

#include "fastlz.h"
#include "validate_json.h"
#include "hardware_configuration.h"


namespace configuration {


    String getResourceStr(const Resource& resource) {
        char decompressed_buffer [resource.decompressed_size+1];
        size_t decompressed_size = fastlz_decompress(resource.data, resource.size, decompressed_buffer, resource.decompressed_size);
        decompressed_buffer[decompressed_size] = 0;
        return String(decompressed_buffer);
    }


    DynamicJsonDocument getResourceJson(const Resource& resource, unsigned size) {
        DynamicJsonDocument document(size ? size : 2 * resource.size);
        deserializeJson(document, getResourceStr(resource));
        return document;
    }


    DynamicJsonDocument getDefautltConfiguration() {
        return getResourceJson(resource_default_config_json);
    }


    DynamicJsonDocument getDefautltFavorites() {
        return getResourceJson(resource_default_favorites_json);
    }


    DynamicJsonDocument getConfigSchema() {
        return getResourceJson(resource_config_schema_json);
    }


    String assertJson(JsonVariant configuration, String name) {
        const auto configSchema = getConfigSchema();
        return validateJson(configuration, configSchema, configSchema[name], ".", getDefautltConfiguration()); 
    }


    String assertConfiguration(JsonVariant configuration) {
        return assertJson(configuration, "main");
    }


    DynamicJsonDocument getVersionInfo() {
        auto versionInfo = getResourceJson(resource_version_json, 1024);
        versionInfo["date"] = __DATE__;
        versionInfo["time"] = __TIME__;
        versionInfo["hardware"] = hardware_configuration.getCode();
        return versionInfo;
    }


    String getConfigurationStr() {
        File configFile = SPIFFS.open(CONFIGURATION_FILENAME, "r");
        if (configFile) {
            String buf = configFile.readString();
            configFile.close();
            return buf;
        }
        return getResourceStr(resource_default_config_json);
    }


    DynamicJsonDocument getConfiguration() {
        DynamicJsonDocument configuration(JSON_CONFIG_BUF_SIZE);
        File configFile = SPIFFS.open(CONFIGURATION_FILENAME, "r");
        if (configFile) {
            String buf = configFile.readString();
            configFile.close();
            DeserializationError err = deserializeJson(configuration, buf);
            if (err != DeserializationError::Ok || assertConfiguration(configuration).length())
                configuration = getDefautltConfiguration();
            // updateModules(); //TOTO
        } else
            configuration = getDefautltConfiguration();
        return configuration;
    }


    DynamicJsonDocument getFavorites() {
        DynamicJsonDocument favorites(JSON_FAVORITES_BUF_SIZE);

        File favoritesFile = SPIFFS.open(FAVORITES_FILENAME, "r");
        if (favoritesFile) {
            String buf = favoritesFile.readString();
            favoritesFile.close();
            DeserializationError err = deserializeJson(favorites, buf);
            if (err != DeserializationError::Ok || assertJson(favorites, "favorites-list").length())
                favorites = getDefautltFavorites();
        } else favorites = getDefautltFavorites();
        return favorites;
    }


    void resetConfiguration() {
        SPIFFS.remove(CONFIGURATION_FILENAME);
        SPIFFS.remove(FAVORITES_FILENAME);
    }


    void setConfiguration(DynamicJsonDocument configuration) {
        char* buf = new char[JSON_CONFIG_BUF_SIZE+1];
        unsigned size = serializeJson(configuration, buf, JSON_CONFIG_BUF_SIZE);
        buf[size] = 0;
        File configFile = SPIFFS.open(CONFIGURATION_FILENAME, "w");
        configFile.println(buf);
        configFile.close();
        delete [] buf;
    }


    void setFavorites(DynamicJsonDocument favorites) {
        char* buf = new char[JSON_FAVORITES_BUF_SIZE+1];
        unsigned size = serializeJson(favorites, buf, JSON_FAVORITES_BUF_SIZE);
        buf[size] = 0;
        File favoritesFile = SPIFFS.open(FAVORITES_FILENAME, "w");
        favoritesFile.println(buf);
        favoritesFile.close();
        delete [] buf;
    }


}
