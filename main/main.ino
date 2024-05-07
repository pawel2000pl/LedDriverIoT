#include <BTAddress.h>
#include <BTAdvertisedDevice.h>
#include <BTScan.h>

#include <WiFi.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>

#include "json.h"
#include "utils.h"
#include "resources.h"
#include "conversions.h"
#include "validate_json.h"
#include "functions.h"

#define CONNECTION_TIMEOUT 5000
#define CONFIGURATION_FILENAME "/configuration.json"
#define JSON_CONFIG_BUF_SIZE (16*1024)
#define MAX_STRING_LENGTH 64
StaticJsonDocument<JSON_CONFIG_BUF_SIZE> configuration;
StaticJsonDocument<JSON_CONFIG_BUF_SIZE> configSchema;

void loadConfiguration() {
  uint8_t decompressed_buffer[default_config_json_decompressed_size+1];
  size_t decompressed_size = fastlz_decompress(default_config_json_data, default_config_json_size, decompressed_buffer, default_config_json_decompressed_size);
  decompressed_buffer[decompressed_size] = 0;
  String buf;
  File configFile = SPIFFS.open(CONFIGURATION_FILENAME, "r");
  if (configFile) {
    buf = configFile.readString();
    configFile.close();
  } else 
      buf = (const char*)decompressed_buffer;
  DeserializationError err = deserializeJson(configuration, buf);
  if (err != DeserializationError::Ok || assertConfiguration().length())
    deserializeJson(configuration, String((const char*)decompressed_buffer));
}


void loadConfigSchema() {
  uint8_t decompressed_buffer[config_schema_json_decompressed_size+1];
  size_t decompressed_size = fastlz_decompress(config_schema_json_data, config_schema_json_size, decompressed_buffer, config_schema_json_decompressed_size);
  decompressed_buffer[decompressed_size] = 0;
  deserializeJson(configSchema, String((const char*)decompressed_buffer));
}


void resetConfiguration() {
  SPIFFS.remove(CONFIGURATION_FILENAME);
  loadConfiguration();
}


void saveConfiguration() {
  char* buf = new char[JSON_CONFIG_BUF_SIZE+1];
  unsigned size = serializeJson(configuration, buf, JSON_CONFIG_BUF_SIZE);
  buf[size] = 0;
  File configFile = SPIFFS.open(CONFIGURATION_FILENAME, "w");
  configFile.println(buf);
  configFile.close();
  delete [] buf;
}

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,254);
IPAddress subnet(255,255,255,0);

WebServer server(80);

IPAddress str2ip(const String& str) {
  uint8_t numbers[4] = {0, 0, 0, 0};
  int j = 0;
  for (int i=0;i<4;i++) {
    while (1) {
      char currChar = str[j++];
      if (currChar < '0' || currChar > '9') break;
      numbers[i] = numbers[i] * 10 + (currChar - '0');
    }
  }
  return IPAddress(numbers[0], numbers[1], numbers[2], numbers[3]);
}

void autoConnectWifi() {
  const auto& wifiConfig = configuration["wifi"];
  const auto& staPriority = wifiConfig["sta_priority"];
  const auto& apConfig = wifiConfig["access_point"];

  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(wifiConfig["hostname"].as<JsonString>().c_str());

  for (int i=0;i<staPriority.size();i++) {
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to ");
    Serial.print(staPriority[i]["ssid"].as<String>());
    Serial.print(" with password ");
    Serial.println(staPriority[i]["password"].as<String>());
    WiFi.begin(staPriority[i]["ssid"].as<String>(), staPriority[i]["password"].as<String>());
    unsigned long long int timeout_time = millis() + CONNECTION_TIMEOUT;
    while (WiFi.status() != WL_CONNECTED && millis() <= timeout_time)
      delay(10);
    if (WiFi.status() == WL_CONNECTED)
      return;
  }
  if (apConfig["enabled"]) {
    Serial.println("Switching to AP-mode");
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(str2ip(apConfig["address"].as<String>()), str2ip(apConfig["gateway"].as<String>()), str2ip(apConfig["subnet"].as<String>()));
    WiFi.softAP(apConfig["ssid"].as<String>(), apConfig["password"].as<String>());
  }
}


void sendConfiguration() {
  char* buf = new char[JSON_CONFIG_BUF_SIZE+1];
  unsigned size = serializeJson(configuration, buf, JSON_CONFIG_BUF_SIZE);
  buf[size] = 0;
  server.send(200, default_config_json_mime_type, buf);
  delete [] buf;
}


void sendError(String message, int code = 400) {
  StaticJsonDocument<1024> messageData;
  char buf[1024];
  messageData["status"] = "error";
  messageData["message"] = message;
  unsigned int size = serializeJson(messageData, buf, 1023);
  buf[size] = 0;
  server.send(code, default_config_json_mime_type, buf);
}


void sendOk() {
  server.send(200, default_config_json_mime_type, "{\"status\": \"ok\"}");
}


String assertConfiguration() {
  return validateJson(configuration, configSchema); 
}


void sendDeserializationError(DeserializationError err) {
    if (err == DeserializationError::EmptyInput) sendError("Empty input", 400);
    else if (err == DeserializationError::IncompleteInput) sendError("Incomplete input", 422);
    else if (err == DeserializationError::InvalidInput) sendError("Invalid input", 422);
    else if (err == DeserializationError::NoMemory) sendError("Out of memory (configuration too long)", 413);
    else if (err == DeserializationError::TooDeep) sendError("Configuration too deep (so it must be invalid)", 413);
    else sendError("Cannot save configuration because no.", 400);
}


void customValidator() {
  String rawData = server.arg("plain");
  StaticJsonDocument<2*JSON_CONFIG_BUF_SIZE> *testJson = new StaticJsonDocument<2*JSON_CONFIG_BUF_SIZE>();
  DeserializationError err = deserializeJson(*testJson, rawData);
  if (err != DeserializationError::Ok) {
    loadConfiguration();
    sendDeserializationError(err);
    delete testJson;
    return;
  }
  const String assertMessage = validateJson((*testJson)["data"], (*testJson)["schema"]);
  delete testJson;
  if (assertMessage != "") {
    sendError(assertMessage, 422);
    return;
  }
  sendOk();
}


void recvConfiguration(bool save=true) {
  String rawData = server.arg("plain");
  DeserializationError err = deserializeJson(configuration, rawData);
  if (err != DeserializationError::Ok) {
    loadConfiguration();
    sendDeserializationError(err);
    return;
  }

  const String assertMessage = assertConfiguration();
  if (assertMessage != "") {
    sendError(assertMessage, 422);
    return;
  }
  if (save)
    saveConfiguration();
  else
    loadConfiguration();
  sendOk();
}


struct {
  float red;
  float green;
  float blue;
  float white;
} outputValues = {0,0,0,0};


void sendOutput() {
  StaticJsonDocument<256> data;
  data["red"] = outputValues.red;
  data["green"] = outputValues.green;
  data["blue"] = outputValues.blue;
  data["white"] = outputValues.white;
  char buf[256];
  int size = serializeJson(data, buf, 255);
  buf[size] = 0;
  server.send(200, default_config_json_mime_type, buf);
}


void setOutput() {
  StaticJsonDocument<256> data;
  String rawData = server.arg("plain");
  DeserializationError err = deserializeJson(data, rawData);
  if (err != DeserializationError::Ok) 
    return sendError("Deserialization error");
  String assertMessage = validateJson(data, configSchema, configSchema["output-values"]);
  if (assertMessage != "")
    sendError(assertMessage, 422);
  outputValues.red = data["red"];
  outputValues.green = data["green"];
  outputValues.blue = data["blue"];
  outputValues.white = data["white"];
  sendOutput();
}


void configureServer() {
  server.on("/", HTTP_GET, [&server](){server.send(200, "text/html", "<meta http-equiv=\"refresh\" content=\"0; url=/index.html\">");});
  server.on("/config.json", HTTP_GET, sendConfiguration);
  server.on("/reset_configuration", HTTP_GET, [&server](){resetConfiguration(); sendOk();});
  server.on("/favicon.ico", HTTP_GET, [&server](){sendDecompressedData(server, resource_favicon_svg.mime_type, resource_favicon_svg.data, resource_favicon_svg.size, resource_favicon_svg.decompressed_size);});
  for (unsigned i=0;i<resources_count;i++)
    server.on(resources[i]->name, HTTP_GET, [i, &server](){sendDecompressedData(server, resources[i]->mime_type, resources[i]->data, resources[i]->size, resources[i]->decompressed_size);});  
  server.on("/config.json", HTTP_POST, [](){recvConfiguration(true);});
  server.on("/assert_config", HTTP_POST, [](){recvConfiguration(false);});
  server.on("/output.json", HTTP_GET, sendOutput);
  server.on("/output.json", HTTP_POST, setOutput);
  server.on("/assert_json", HTTP_POST, customValidator);
  server.begin();
}


void setup() {
  Serial.begin(115200);

  SPIFFS.begin(true);
  loadConfigSchema();
  loadConfiguration();
  autoConnectWifi(); 
  configureServer();
}

void loop() {
  server.handleClient();
}
