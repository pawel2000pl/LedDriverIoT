#include <BTAddress.h>
#include <BTAdvertisedDevice.h>
#include <BTScan.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <functional>
#include <list>

#include "json.h"
#include "utils.h"
#include "resources.h"
#include "conversions.h"
#include "validate_json.h"
#include "functions.h"

#define CONNECTION_TIMEOUT 15000
#define CONFIGURATION_FILENAME "/configuration.json"
#define JSON_CONFIG_BUF_SIZE (16*1024)
#define MAX_STRING_LENGTH 64
StaticJsonDocument<JSON_CONFIG_BUF_SIZE> configuration;
StaticJsonDocument<JSON_CONFIG_BUF_SIZE> configSchema;
StaticJsonDocument<JSON_CONFIG_BUF_SIZE> scannedNetworks;
std::list<std::function<void()>> taskQueue;

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


void scanNetworks() {
  delay(100);
  Serial.println("Scanning networks");
  int n = WiFi.scanNetworks();
  scannedNetworks.clear();
  for (int i=0;i<n;i++)
    scannedNetworks.add(WiFi.SSID(i));
  WiFi.scanDelete();
  delay(100);
}


void sendNetworks() {
  char* buf = new char[JSON_CONFIG_BUF_SIZE+1];
  unsigned size = serializeJson(scannedNetworks, buf, JSON_CONFIG_BUF_SIZE);
  buf[size] = 0;
  server.send(200, default_config_json_mime_type, buf);
  delete [] buf;
}


bool connectToNetwork(String ssid, String password) {
  if (WiFi.status() == WL_CONNECTED)
    WiFi.disconnect();
  delay(100);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  unsigned long long int timeout_time = millis() + CONNECTION_TIMEOUT;
  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED && WiFi.status() != WL_CONNECT_FAILED && millis() <= timeout_time)
        delay(10);
  return WiFi.status() == WL_CONNECTED;
}


void autoConnectWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
  }

  scanNetworks();

  const auto& wifiConfig = configuration["wifi"];
  const auto& staPriority = wifiConfig["sta_priority"];
  const auto& apConfig = wifiConfig["access_point"];

  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(wifiConfig["hostname"].as<JsonString>().c_str());

  for (int i=0;i<staPriority.size();i++) {
    WiFi.mode(WIFI_STA);
    if (connectToNetwork(staPriority[i]["ssid"].as<String>(), staPriority[i]["password"].as<String>()))
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
  const String assertMessage = testJson->containsKey("type") ? validateJson((*testJson)["data"], configSchema, configSchema[(*testJson)["type"].as<String>()]) : validateJson((*testJson)["data"], (*testJson)["schema"]);
  delete testJson;
  if (assertMessage != "") {
    sendError(assertMessage, 422);
    return;
  }
  sendOk();
}


void connectToNetworkEndpoint() {
  String rawData = server.arg("plain");
  StaticJsonDocument<768> data;
  DeserializationError err = deserializeJson(data, rawData);
  if (err != DeserializationError::Ok) {
    sendDeserializationError(err);
    return;
  }
  const String assertMessage = validateJson(data, configSchema, configSchema["wifi-entry"]);
  if (assertMessage != "") {
    sendError(assertMessage, 422);
    return;
  }
  sendOk();
  const String ssid = data["ssid"].as<String>();
  const String password = data["password"].as<String>();
  taskQueue.push_back([ssid, password](){delay(100); connectToNetwork(ssid, password);});
}


void autoScanWifiEndpoint() {
  sendOk(); 
  taskQueue.push_back([](){delay(100); autoConnectWifi();});
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
} colorValues = {0,0,0,0};


void sendColors() {
  StaticJsonDocument<256> data;
  data["red"] = colorValues.red;
  data["green"] = colorValues.green;
  data["blue"] = colorValues.blue;
  data["white"] = colorValues.white;
  char buf[256];
  int size = serializeJson(data, buf, 255);
  buf[size] = 0;
  server.send(200, default_config_json_mime_type, buf);
}


void setColors() {
  StaticJsonDocument<256> data;
  String rawData = server.arg("plain");
  DeserializationError err = deserializeJson(data, rawData);
  if (err != DeserializationError::Ok) 
    return sendError("Deserialization error");
  String assertMessage = validateJson(data, configSchema, configSchema["color-values"]);
  if (assertMessage != "")
    sendError(assertMessage, 422);
  colorValues.red = data["red"];
  colorValues.green = data["green"];
  colorValues.blue = data["blue"];
  colorValues.white = data["white"];
  sendColors();
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
  server.on("/color.json", HTTP_GET, sendColors);
  server.on("/color.json", HTTP_POST, setColors);
  server.on("/assert_json", HTTP_POST, customValidator);
  server.on("/networks.json", HTTP_GET, sendNetworks);
  server.on("/connect_to", HTTP_POST, connectToNetworkEndpoint);
  server.on("/refresh_networks", HTTP_GET, autoScanWifiEndpoint);
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
  for (auto fun: taskQueue) 
    fun();
  taskQueue.clear();
  if (WiFi.status() != WL_CONNECTED)
    autoConnectWifi(); 
}
