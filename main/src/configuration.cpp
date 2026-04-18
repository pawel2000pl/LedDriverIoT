#include "configuration.h"

#include <FS.h>
#include <SPIFFS.h>

#include "lib/littlefs/LittleFS.h"
#include "memory_stream.h"
#include "validate_json.h"
#include "hardware_configuration.h"
#include "logs.h"


namespace configuration {

    JsonDocument getResourceJson(const Resource& resource, unsigned size) {
        JsonDocument document;
        deserializeJson(document, resource.data);
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
        versionInfo["hardware"] = hardware::configuration->getCode();
        versionInfo["resources_sha"] = RESOURCES_SHA1;
        return versionInfo;
    }


    void removeFile(const String& filename) {
        LittleFS.remove(filename);
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


    unsigned getFileBinLITTLEFS(const String& filename, std::vector<unsigned char>& buf) {
        File file = LittleFS.open(filename);
        if (file.available()) {
            buf.resize(file.size());
            file.readBytes((char*)buf.data(), file.size());
            file.close();
            return buf.size();
        }
        return 0;
    }


    unsigned getFileBinSPIFFS(const String& filename, std::vector<unsigned char>& buf) {
        File file = SPIFFS.open(filename);
        if (file.available()) {
            buf.resize(file.size());
            file.readBytes((char*)buf.data(), file.size());
            file.close();
            return buf.size();
        }
        return 0;
    }


    unsigned getFileBin(const String& filename, unsigned char* buf, unsigned max_size) {
        File file = LittleFS.open(filename);
        if (file.available()) {
            unsigned result = file.readBytes((char*)buf, std::min(max_size, file.size()));
            file.close();
            return result;
        }
        return 0;
    }


    void serializeToFile(const String filename, const JsonDocument& data) {        
        std::size_t size = measureJson(data);
        std::vector<unsigned char> buf(size * 1.2f + 3);
        size = serializeJson(data, buf.data(), buf.size());
        saveFile(filename, buf.data(), size);
    }


    bool saveFile(const String& filename, const unsigned char* content, unsigned length) {
        const size_t CHUNK_SIZE = 256;
        const size_t TRIALS = 256;
        uint8_t verifyBuf[CHUNK_SIZE];
        
        logs::logger.printf("Saving file %s\n", filename.c_str());
        for (int attempt = 1; attempt <= TRIALS; ++attempt) {
            bool success = true;

            File file = LittleFS.open(filename, FILE_WRITE);
            if (!file) {
                logs::logger.printf("Attempt %d: open for write failed\n", attempt);
                success = false;
            } else {
                size_t writtenTotal = 0;
                while (writtenTotal < length) {
                    size_t toWrite = min(CHUNK_SIZE, length - writtenTotal);
                    size_t written = file.write(content + writtenTotal, toWrite);
                    if (written != toWrite) {
                        logs::logger.printf("Attempt %d: write error at offset %u\n", attempt, writtenTotal);
                        success = false;
                        break;
                    }
                    writtenTotal += written;
                }
                file.close();
            }

            if (success) {
                File file = LittleFS.open(filename, FILE_READ);
                if (!file) {
                    logs::logger.printf("Attempt %d: open for read failed\n", attempt);
                    success = false;
                } else {
                    size_t readTotal = 0;
                    while (readTotal < length) {
                        size_t toRead = min(CHUNK_SIZE, length - readTotal);
                        size_t readBytes = file.read(verifyBuf, toRead);
                        if (readBytes != toRead) {
                            logs::logger.printf("Attempt %d: read error at offset %u\n", attempt, readTotal);
                            success = false;
                            break;
                        }
                        if (memcmp(content + readTotal, verifyBuf, toRead) != 0) {
                            logs::logger.printf("Attempt %d: verify mismatch at offset %u\n", attempt, readTotal);
                            success = false;
                            break;
                        }
                        readTotal += readBytes;
                    }
                    file.close();
                }
            }

            if (success) {
                logs::logger.println("success");
                return true;
            }

            if (attempt < 16) {
                delay(5);
            } else {
                logs::logger.printf("Final failure after %d attempts: %s\n", attempt, filename.c_str());
                return false;
            }
        }
        return false;
    }


    bool saveFile(const String& filename, const char* content) {
        unsigned length = -1;
        while (content[++length]);
        return saveFile(filename, (const unsigned char*)content, length);
    }


    bool saveFile(const String& filename, const String& content) {
        return saveFile(filename, (const unsigned char*)content.c_str(), content.length());
    }

    
    std::unique_ptr<Stream> getReadStream(const String& filename, const Resource* default_resource) {
        std::unique_ptr<Stream> file = std::make_unique<File>(LittleFS.open(filename, FILE_READ));
        if (file->available()) 
            return file;
        if (default_resource)
            return std::make_unique<MemoryStream>(default_resource->data, default_resource->size);
        return NULL;
    }


    std::unique_ptr<Stream> getConfigStream() {
        return getReadStream(CONFIGURATION_FILENAME, &resource_default_config_json);
    }


    JsonDocument getConfiguration() {
        JsonDocument configuration;
        auto stream = getConfigStream();
        DeserializationError err = deserializeJson(configuration, *stream);
        if (err != DeserializationError::Ok || assertConfiguration(configuration).length())
            configuration = getDefautltConfiguration();
        return configuration;
    }


    std::unique_ptr<Stream> getFavoritesStream() {
        return getReadStream(FAVORITES_FILENAME, &resource_default_favorites_json);
    }


    JsonDocument getFavorites() {
        JsonDocument favorites;
        auto stream = getFavoritesStream();
        DeserializationError err = deserializeJson(favorites, *stream);
        if (err != DeserializationError::Ok || assertJson(favorites, "favorites-list").length())
            favorites = getDefautltFavorites();
        return favorites;
    }


    std::unique_ptr<Stream> getAnimationsStream() {
        return getReadStream(ANIMATIONS_FILENAME, &resource_default_animations_json);
    }


    JsonDocument getAnimations() {
        JsonDocument animations;
        auto stream = getAnimationsStream();
        DeserializationError err = deserializeJson(animations, *stream);
        if (err != DeserializationError::Ok || assertJson(animations, "animations-list").length())
            animations = getDefautltAnimations();
        return animations;
    }


    void deleteCert() {
        removeFile(CERT_KEY_FILE_NAME);
        removeFile(CERT_PUB_FILE_NAME);
    }
    

    void rewriteFilesystem(bool fromSPIFFS=false) {
        logs::logger.println("Rewritting filesystem.");

        std::vector<String> filenames = {
            CONFIGURATION_FILENAME,
            FAVORITES_FILENAME,
            ANIMATIONS_FILENAME,
            CERT_KEY_FILE_NAME,
            CERT_PUB_FILE_NAME
        };

        const auto readFunction = fromSPIFFS ? getFileBinSPIFFS : getFileBinLITTLEFS;

        unsigned fileCount = filenames.size();
        std::vector<std::vector<unsigned char>> buffers;
        buffers.resize(fileCount);

        for (unsigned i=0;i<fileCount;i++) {
            delay(20);
            readFunction(filenames[i], buffers[i]);
        }

        delay(20);
        if (fromSPIFFS) {
            SPIFFS.end();
        } else {
            LittleFS.end();
        }
        delay(20);
        LittleFS.begin(true);
        delay(20);
        LittleFS.format();

        for (unsigned i=0;i<fileCount;i++) {
            delay(20);
            if (buffers[i].size()) 
                saveFile(filenames[i], buffers[i].data(), buffers[i].size());
        }
        delay(20);     
    }


    void mountFileSystem() {
        bool success_read = false;
        bool many_trials = false;
        for (int i=0;i<16;i++) {
	        if (LittleFS.begin(false)) {
                success_read = true;
                break;
            }
            logs::logger.printf("LittleFS failed (%d/16)\n",i+1);
            many_trials = true;
            delay(100);
        }
        if (!success_read && SPIFFS.begin(false)) {
            logs::logger.println("SPIFFS succeeded.");
            rewriteFilesystem(true);
            logs::logger.println("Rewritted filesystem to LittleFS.");
        } else if (!success_read) {
            LittleFS.begin(true);
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
        LittleFS.format();
    }


    void setConfiguration(JsonDocument configuration) {
        serializeToFile(CONFIGURATION_FILENAME, configuration);
    }


    void setFavorites(JsonDocument favorites) {
        serializeToFile(FAVORITES_FILENAME, favorites);
    }


    void setAnimations(JsonDocument animations) {
        serializeToFile(ANIMATIONS_FILENAME, animations);
    }


}
