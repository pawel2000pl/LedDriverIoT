#include "configuration.h"

#include <FS.h>
#include <SPIFFS.h>

#include "lib/fastlz/fastlz.h"

#include "validate_json.h"
#include "hardware_configuration.h"


namespace configuration {


    String getResourceStr(const Resource& resource) {
        char decompressed_buffer [resource.decompressed_size+1];
        size_t decompressed_size = fastlz_decompress(resource.data, resource.size, decompressed_buffer, resource.decompressed_size);
        decompressed_buffer[decompressed_size] = 0;
        return String(decompressed_buffer);
    }


    JsonDocument getResourceJson(const Resource& resource, unsigned size) {
        JsonDocument document;
        deserializeJson(document, getResourceStr(resource));
        return document;
    }


    JsonDocument getDefautltConfiguration() {
        return getResourceJson(resource_default_config_json);
    }


    JsonDocument getDefautltFavorites() {
        return getResourceJson(resource_default_favorites_json);
    }


    JsonDocument getDefautltAnimations() {
        return getResourceJson(resource_default_animations_json);
    }


    JsonDocument getConfigSchema() {
        return getResourceJson(resource_config_schema_json);
    }


    String assertJson(JsonVariant configuration, String name) {
        const auto configSchema = getConfigSchema();
        return validateJson(configuration, configSchema, configSchema[name], ".", getDefautltConfiguration()); 
    }


    String assertConfiguration(JsonVariant configuration) {
        return assertJson(configuration, "main");
    }


    JsonDocument getVersionInfo() {
        auto versionInfo = getResourceJson(resource_version_json, 1024);
        versionInfo["date"] = __DATE__;
        versionInfo["time"] = __TIME__;
        versionInfo["hardware"] = hardware_configuration.getCode();
        versionInfo["resources_sha"] = RESOURCES_SHA1;
        return versionInfo;
    }


    void removeFile(const String& filename) {
        SPIFFS.remove(filename);
    }


    unsigned checkSum(const unsigned char* ptr, unsigned size, unsigned d) {
        unsigned long long int buf = 1;
        const unsigned char* endloop = ptr + size;
        for (auto i = ptr; i < endloop; i++) {
            buf = ((buf << 8) | *i);
            if (buf >> 56) buf %= d;
        }
        return (buf < d) ? buf % d : buf;
    }


    std::vector<unsigned char>* getFileBin(const String& filename) {
        File file = SPIFFS.open(filename);
        if (file) {
            std::vector<unsigned char>* result = new std::vector<unsigned char>();
            result->resize(file.size());
            file.read(result->data(), file.size());
            file.close();
            return result;
        }
        return new std::vector<unsigned char>();
    }


    String getFileStr(const String& filename) {
        File file = SPIFFS.open(filename);
        if (file) {
            String buf = file.readString();
            file.close();
            return buf;
        }
        return "";
    }


    void saveFile(const String& filename, const unsigned char* content, unsigned length) {
        File file = SPIFFS.open(filename, FILE_WRITE);
        file.write(content, length);
        file.close();
    }


    void saveFile(const String& filename, const char* content) {
        unsigned length = -1;
        while (content[++length]);
        saveFile(filename, (const unsigned char*)content, length);
    }


    void saveFile(const String& filename, const String& content) {
        saveFile(filename, (const unsigned char*)content.c_str(), content.length());
    }


    String getConfigurationStr() {
        String buf = getFileStr(CONFIGURATION_FILENAME);
        return buf == "" ? getResourceStr(resource_default_config_json) : buf;
    }


    JsonDocument getConfiguration() {
        JsonDocument configuration;
        String buf = getConfigurationStr();
        DeserializationError err = deserializeJson(configuration, buf);
        if (err != DeserializationError::Ok || assertConfiguration(configuration).length())
            configuration = getDefautltConfiguration();
        return configuration;
    }


    JsonDocument getFavorites() {
        JsonDocument favorites;
        String buf = getFileStr(FAVORITES_FILENAME);
        DeserializationError err = deserializeJson(favorites, buf);
        if (err != DeserializationError::Ok || assertJson(favorites, "favorites-list").length())
            favorites = getDefautltFavorites();
        return favorites;
    }


    JsonDocument getAnimations() {
        JsonDocument animations;
        String buf = getFileStr(ANIMATIONS_FILENAME);
        DeserializationError err = deserializeJson(animations, buf);
        if (err != DeserializationError::Ok || assertJson(animations, "animations-list").length())
            animations = getDefautltAnimations();
        return animations;
    }


    void deleteCert() {
        removeFile(CERT_KEY_FILE_NAME);
        removeFile(CERT_PUB_FILE_NAME);
    }
    

    void rewriteFilesystem() {
        std::vector<unsigned char>* configuration = SPIFFS.exists(CONFIGURATION_FILENAME) ? getFileBin(CONFIGURATION_FILENAME) : NULL;
        std::vector<unsigned char>* favorites = SPIFFS.exists(FAVORITES_FILENAME) ? getFileBin(FAVORITES_FILENAME) : NULL;
        std::vector<unsigned char>* animations = SPIFFS.exists(ANIMATIONS_FILENAME) ? getFileBin(ANIMATIONS_FILENAME) : NULL;        
        std::vector<unsigned char>* cert_key = SPIFFS.exists(CERT_KEY_FILE_NAME) ? getFileBin(CERT_KEY_FILE_NAME) : NULL;
        std::vector<unsigned char>* cert_pub = SPIFFS.exists(CERT_PUB_FILE_NAME) ? getFileBin(CERT_PUB_FILE_NAME) : NULL;    
        
        SPIFFS.format();

        if (configuration) saveFile(CONFIGURATION_FILENAME, configuration->data(), configuration->size());
        if (favorites) saveFile(FAVORITES_FILENAME, favorites->data(), favorites->size());
        if (animations) saveFile(ANIMATIONS_FILENAME, animations->data(), animations->size());
        if (cert_key) saveFile(CERT_KEY_FILE_NAME, cert_key->data(), cert_key->size());
        if (cert_pub) saveFile(CERT_PUB_FILE_NAME, cert_pub->data(), cert_pub->size());
        
        if (configuration) delete configuration;
        if (favorites) delete favorites;
        if (animations) delete animations;
        if (cert_key) delete cert_key;
        if (cert_pub) delete cert_pub;
    }


    void mountFileSystem() {
        bool success_read = false;
        bool many_trials = false;
        for (int i=0;i<16;i++) {
	        if (SPIFFS.begin(false)) {
                success_read = true;
                break;
            }
            many_trials = true;
            delay(100);
        }
        if (!success_read) {
            SPIFFS.begin(true);
        } else if (many_trials) {
            rewriteFilesystem();
        }
    }


    void loadConfiguration() {
        JsonDocument config = getConfiguration();
        if (assertConfiguration(config).length())
            setConfiguration(getDefautltConfiguration());
        else
            setConfiguration(config);
    }


    void init() {
        mountFileSystem();
        loadConfiguration();
    }


    void resetConfiguration() {
        SPIFFS.format();
    }


    void setConfiguration(JsonDocument configuration) {
        char* buf = new char[JSON_CONFIG_BUF_SIZE+1];
        unsigned size = serializeJson(configuration, buf, JSON_CONFIG_BUF_SIZE);
        buf[size] = 0;
        saveFile(CONFIGURATION_FILENAME, (unsigned char*)buf, size);
        delete [] buf;
    }


    void setFavorites(JsonDocument favorites) {
        char* buf = new char[JSON_FAVORITES_BUF_SIZE+1];
        unsigned size = serializeJson(favorites, buf, JSON_FAVORITES_BUF_SIZE);
        saveFile(FAVORITES_FILENAME, (unsigned char*)buf, size);
        delete [] buf;
    }


    void setAnimations(JsonDocument animations) {
        char* buf = new char[JSON_ANIMATIONS_BUF_SIZE+1];
        unsigned size = serializeJson(animations, buf, JSON_ANIMATIONS_BUF_SIZE);
        saveFile(ANIMATIONS_FILENAME, (unsigned char*)buf, size);
        delete [] buf;
    }


}
