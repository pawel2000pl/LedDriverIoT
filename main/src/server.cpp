#include "server.h"

#include <sstream>

#include "wifi.h"
#include "fastlz.h"
#include "constrain.h"
#include "resources.h"
#include "configuration.h"


namespace server {


    std::vector<unsigned char>* keyBuf = NULL;
    std::vector<unsigned char>* certBuf = NULL;
    httpsserver::SSLCert* cert;
    httpsserver::HTTPSServer* secureServer;
    httpsserver::HTTPServer* insecureServer;
    bool captivePortalEnabled = false;


    void updateConfiguration(const JsonVariantConst& configuration) {
        captivePortalEnabled = configuration["wifi"]["access_point"]["captive"].as<bool>();
    }


    void extractMacroDate(int &day, int &month, int &year) {
        std::string dateStr = __DATE__;
        std::istringstream iss(dateStr);
        std::string monthStr;
        std::string dayStr;
        
        iss >> monthStr >> dayStr >> year;

        if (monthStr == "Jan") month = 1;
        else if (monthStr == "Feb") month = 2;
        else if (monthStr == "Mar") month = 3;
        else if (monthStr == "Apr") month = 4;
        else if (monthStr == "May") month = 5;
        else if (monthStr == "Jun") month = 6;
        else if (monthStr == "Jul") month = 7;
        else if (monthStr == "Aug") month = 8;
        else if (monthStr == "Sep") month = 9;
        else if (monthStr == "Oct") month = 10;
        else if (monthStr == "Nov") month = 11;
        else if (monthStr == "Dec") month = 12;

        day = std::stoi(dayStr);
    }


    std::string buildDate(int day, int month, int year) {
        char buf[16];
        int len = sprintf(buf, "%04d%02d%02d000000", year, month, day);
        buf[len] = 0;
        return std::string(buf);
    }


    std::string getCertValidFromDate() {
        int day, month, year;
        extractMacroDate(day, month, year);
        return buildDate(day, month, year);
    }


    std::string getCertValidUntilDate() {
        int day, month, year;
        extractMacroDate(day, month, year);
        return buildDate(day, month, year+250);
    }


    httpsserver::SSLCert* generateCert() {
        httpsserver::SSLCert* newCert = new httpsserver::SSLCert();
        std::string fromDate = getCertValidFromDate();
        std::string untilDate = getCertValidUntilDate();
        httpsserver::createSelfSignedCert(
            *newCert,
            httpsserver::KEYSIZE_1024,
            "CN=leddriver.local,O=PawelBielecki,C=PL",
            fromDate.c_str(),
            untilDate.c_str()
        );
        configuration::saveFile(CERT_PUB_FILE_NAME, newCert->getCertData(), newCert->getCertLength());
        configuration::saveFile(CERT_KEY_FILE_NAME, newCert->getPKData(), newCert->getPKLength());
        return newCert;
    }


    httpsserver::SSLCert* loadCert() {
        if (certBuf) {delete certBuf; certBuf = NULL;}
        if (keyBuf) {delete keyBuf; keyBuf = NULL;}
        certBuf = new std::vector<unsigned char>(configuration::getFileBin(CERT_PUB_FILE_NAME));
        keyBuf = new std::vector<unsigned char>(configuration::getFileBin(CERT_KEY_FILE_NAME));
        if (certBuf->size() && keyBuf->size()) {
            return new httpsserver::SSLCert(
                certBuf->data(), certBuf->size(),
                keyBuf->data(), keyBuf->size()
            );
        }
        return generateCert();
    }


    void configure() {
        cert = loadCert();

        secureServer = new httpsserver::HTTPSServer(cert);
        insecureServer = new httpsserver::HTTPServer();

        ResourceNode* staticNode  = new ResourceNode("", "GET", &sendResource);
        secureServer->setDefaultNode(staticNode);
        insecureServer->setDefaultNode(staticNode);
    }


    void start() {
        secureServer->start();
        insecureServer->start();

        if (!secureServer->isRunning()) {
            configuration::removeFile(CERT_PUB_FILE_NAME);
            configuration::removeFile(CERT_KEY_FILE_NAME);
        }
    }


    void loop() {
        secureServer->loop();
        insecureServer->loop();
    }


    void addCallback(const char* address, const char* method, const CallbackFunction* callback) {
        ResourceNode* node = new ResourceNode(address, method, callback);
        secureServer->registerNode(node);
        insecureServer->registerNode(node);
    }


    void sendJson(HTTPResponse* res, const JsonVariantConst& data, unsigned bufSize, int costatusCode) {
        char* buf = new char[bufSize];
        unsigned int size = serializeJson(data, buf, bufSize-1);
        buf[size] = 0;
        char size_str[24];
        res->setHeader("Cache-Control", "no-cache");
        res->setHeader("Content-Type", "application/json");
        res->setHeader("Content-Length", itoa(size, size_str, 10));
        res->setStatusCode(costatusCode);
        res->write((uint8_t*)buf, size);
        delete [] buf;
    }


    void sendError(HTTPResponse* res, String message, int code = 400) {
        StaticJsonDocument<1024> messageData;
        messageData["status"] = "error";
        messageData["message"] = message;
        char buf[1024];
        unsigned int size = serializeJson(messageData, buf, 1023);
        sendJson(res, messageData, 1024, code);
    }


    void sendOk(HTTPResponse* res) {
        const char* buf = "{\"status\": \"ok\"}";
        char size_str[24];
        int size = strlen(buf);
        res->setHeader("Cache-Control", "no-cache");
        res->setHeader("Content-Type", "application/json");
        res->setHeader("Content-Length", itoa(size, size_str, 10));
        res->setStatusCode(200);
        res->write((uint8_t*)buf, size);
    }


    void sendCacheControlHeader(HTTPResponse* res) {
        int minAge = 432000; 
        int maxAge = 604800; 
        int randomAge = random(minAge, maxAge + 1);
        String headerValue = "max-age=" + String(randomAge);
        res->setHeader("Cache-Control", headerValue.c_str());
    }


    int sendDecompressedData(HTTPResponse* res, const Resource& resource, int statusCode) {
		uint8_t* decompressed_buffer = new uint8_t[resource.decompressed_size];
		size_t decompressed_size = fastlz_decompress(resource.data, resource.size, decompressed_buffer, resource.decompressed_size);
		if (decompressed_size == 0) {
				sendError(res, "Decompression error");
				delete [] decompressed_buffer;
				return 0;
		}
		if (statusCode >= 200 && statusCode < 300) 
            sendCacheControlHeader(res);
        char size_str[24];
        res->setHeader("Content-Type", resource.mime_type);
        res->setHeader("Content-Length", itoa(decompressed_size, size_str, 10));
        res->setHeader("ETag", resource.etag);
        res->setStatusCode(statusCode);
		res->write((uint8_t*)decompressed_buffer, decompressed_size);
		delete [] decompressed_buffer;
		return 1;
    }


    bool sendDeserializationError(HTTPResponse* res, DeserializationError err) {
        if (err == DeserializationError::Ok) return false;
        else if (err == DeserializationError::EmptyInput) sendError(res, "Empty input", 400);
        else if (err == DeserializationError::IncompleteInput) sendError(res, "Incomplete input", 422);
        else if (err == DeserializationError::InvalidInput) sendError(res, "Invalid input", 422);
        else if (err == DeserializationError::NoMemory) sendError(res, "Out of memory (content too long)", 413);
        else if (err == DeserializationError::TooDeep) sendError(res, "Data structure is too deep", 413);
        else sendError(res, "Unknown error in deserialization", 400);
        return true;
    }


    void sendResource(HTTPRequest* req, HTTPResponse* res) {
        const Resource& resource = getResourceByName(req->getRequestString().c_str(), &resource_not_found_html);
        bool notFound = resource.name == resource_not_found_html.name; //yes, we can compare const char* in this context
        bool useCaptive = captivePortalEnabled && wifi::isAP();
        if (notFound && useCaptive) {
            String apAddress = wifi::getApAddress();
            res->setHeader("Location", "http://" + std::string(apAddress.c_str()) + "/");
        }
        if ((!notFound) && (req->getHeader("If-None-Match").find(resource.etag) != std::string::npos)) {
            sendCacheControlHeader(res);
            res->setHeader("ETag", resource.etag);
            res->setStatusCode(304);
            return;
        }
        sendDecompressedData(res, resource, notFound ? (useCaptive ? 302 : 404) : 200); 
    }


    std::vector<char> readBuffer(HTTPRequest* req, bool addZero) {
        std::vector<char> result;
        std::string content_length = req->getHeader("Content-Length");
        int buf_size = constrain(atoi(content_length.c_str()), 0, MAX_POST_SIZE);
        if (!buf_size) buf_size = MAX_POST_SIZE;
        result.resize(buf_size + !!addZero);
        char* result_data = result.data();
        unsigned size = 0;
        while (!req->requestComplete() && size < buf_size) {
            size += req->readChars(result_data + size, buf_size-size);
        }
        if (addZero && (size == 0 || result[size-1] != 0)) 
            result_data[size++] = 0;
        result.resize(size);
        if (3*size < 2*buf_size)
            result.shrink_to_fit();
        return std::move(result);
    }


    bool readJson(HTTPRequest* req, HTTPResponse* res, JsonDocument& json, const String& assertEntryName) {
        std::vector<char> rawData = server::readBuffer(req, true);
        DeserializationError err = deserializeJson(json, (const char*)rawData.data());
        if (sendDeserializationError(res, err)) 
            return false;
        if (assertEntryName != "") {
            const String assertMessage = configuration::assertJson(json, assertEntryName);
            if (assertMessage != "") {
                sendError(res, assertMessage, 422);
                return false;
            }
        }
        return true;
    }

}
