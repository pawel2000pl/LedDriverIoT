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
#include "resources.h"
#include "conversions.h"
#include "validate_json.h"
#include "functions.h"
#include "fastlz.h"

#define CONNECTION_TIMEOUT 15000
#define CONFIGURATION_FILENAME "/configuration.json"
#define JSON_CONFIG_BUF_SIZE (16*1024)
#define MAX_STRING_LENGTH 64
StaticJsonDocument<JSON_CONFIG_BUF_SIZE> configuration;
StaticJsonDocument<JSON_CONFIG_BUF_SIZE> configSchema;
StaticJsonDocument<JSON_CONFIG_BUF_SIZE> scannedNetworks;
std::list<std::function<void()>> taskQueue;


struct RGBW {
  float red;
  float green;
  float blue;
  float white;

  float& operator[] (const int i) {
    return *(&red + i);
  }
};


int sendDecompressedData(WebServer& server, const char* content_type, const void* compressed_buffer, size_t compressed_size, size_t max_decompressed_size) {
    uint8_t* decompressed_buffer = new uint8_t[max_decompressed_size+1];
    size_t decompressed_size = fastlz_decompress(compressed_buffer, compressed_size, decompressed_buffer, max_decompressed_size);
    if (decompressed_size == 0) {
        Serial.println("Błąd dekompresji");
        delete [] decompressed_buffer;
        return 0;
    }
    decompressed_buffer[decompressed_size] = 0;
    server.send(200, content_type, (const char*)decompressed_buffer);
    delete [] decompressed_buffer;
    return 1;
}


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
  WiFi.mode(WIFI_STA);
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


void openAccessPoint() {
  Serial.println("Switching to AP-mode");
  if (WiFi.status() == WL_CONNECTED)
    WiFi.disconnect();
  const auto& apConfig = configuration["wifi"]["access_point"];
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(str2ip(apConfig["address"].as<String>()), str2ip(apConfig["gateway"].as<String>()), str2ip(apConfig["subnet"].as<String>()));
  WiFi.softAP(apConfig["ssid"].as<String>(), apConfig["password"].as<String>(), 1, apConfig["hidden"].as<bool>());
}


bool networkIsInScanned(String name) {
  const JsonArrayConst& list = scannedNetworks.as<JsonArrayConst>();
  int size = list.size();
  for (int i=0;i<size;i++)
    if (list[i].as<String>() == name)
      return true;
  return false;
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
    const auto& entry = staPriority[i];
    String ssid = staPriority[i]["ssid"].as<String>();
    if (entry["hidden"].as<bool>() || networkIsInScanned(ssid)) {
      if (connectToNetwork(ssid, staPriority[i]["password"].as<String>()))
        return;
    }
  }
  if (apConfig["enabled"])
      openAccessPoint();
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


void openAccessPointEndpoint() {
  sendOk(); 
  taskQueue.push_back([](){delay(100); openAccessPoint();});
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


RGBW colorValues = {0,0,0,0};
RGBW filteredValues = {0,0,0,0};
float lastKnobValues[4] = {0,0,0,0};
float outputValues[4] = {0,0,0,0};
bool knobMode = true;


void updateOutputs() {
  const auto& transistorConfiguration = configuration["hardware"]["transistorConfiguration"];
  char key[] = {'o', 'u', 't', 'p', 'u', 't', ' ', '0', 0};
  int idx;
  for (idx=0;key[idx]!='0';idx++);
  for (int i=0;i<4;i++) {
    key[idx] = '0' + i;
    unsigned selectedChannel = (unsigned)transistorConfiguration[(const char*)key].as<int>();
    outputValues[i] = selectedChannel <= 4 ? filteredValues[selectedChannel] : 0;
  }
  //TODO: update phisical outputs
}


void updateFilteredValues() {
  const auto& filters = configuration["filters"];
  const auto& channelFilters = filters["outputFilters"];
  FloatFunction global = mixFilterFunctions(filters["globalOutputFilters"].as<JsonArrayConst>());
  FloatFunction red = mixFilterFunctions(channelFilters["red"].as<JsonArrayConst>());
  FloatFunction green = mixFilterFunctions(channelFilters["green"].as<JsonArrayConst>());
  FloatFunction blue = mixFilterFunctions(channelFilters["blue"].as<JsonArrayConst>());
  FloatFunction white = mixFilterFunctions(channelFilters["white"].as<JsonArrayConst>());
  filteredValues.red = red(global(colorValues.red));
  filteredValues.green = green(global(colorValues.green));
  filteredValues.blue = blue(global(colorValues.blue));
  filteredValues.white = white(global(colorValues.white));
  updateOutputs();
}


void setFromKnobs(const float values[4]) {
  if (!knobMode) {
    float epsilon = configuration["hardware"]["knobActivateDelta"].as<float>();
    for (int i=0;i<4;i++)
      if (abs(values[i] - lastKnobValues[i]) > epsilon)
        knobMode = true;
    if (!knobMode) return;
  }
  for (int i=0;i<4;i++)
    lastKnobValues[i] = values[i];

  const auto& bias = configuration["hardware"]["bias"];
  float biasUp = bias["up"].as<float>();
  float biasDown = bias["down"].as<float>();
  const auto applyBias = [biasUp, biasDown](float x) { return (x - biasDown) / (1.f - biasUp - biasDown); };

  const float fixedValues[] = {applyBias(values[0]), applyBias(values[1]), applyBias(values[2]), applyBias(values[3]), 0, 1};
  const String knobMode = configuration["channels"]["knobMode"];
  const char* colorspaces[] = {"hsv", "hsl", "rgb"};
  const char* channels[][4] = {{"hue", "saturation", "value", "white"}, {"hue", "saturation", "lightness", "white"}, {"red", "green", "blue", "white"}};
  std::function<void(float, float, float, float&, float&, float&)> conversionFunctions[] = {hsvToRgb, hslToRgb, rgbToRgb};
  auto conversionFunction = conversionFunctions[0];
  const char** channelsInCurrentColorspace = channels[0];
  for (int i=0;i<3;i++)
    if (knobMode == colorspaces[i]) {
      channelsInCurrentColorspace = channels[i];
      conversionFunction = conversionFunctions[i];
    }
  const auto& potentionemterConfiguration = configuration["hardware"]["potentionemterConfiguration"];
  const auto& filters = configuration["filters"];
  const auto& channelFilters = filters["inputFilters"];
  const auto globalFilter = mixFilterFunctions(filters["globalOutputFilters"].as<JsonArrayConst>());
  float outputChannels[4];
  for (int i=0;i<4;i++) {
    unsigned selectedPotentiometer = (unsigned)potentionemterConfiguration[channelsInCurrentColorspace[i]].as<int>();
    const auto filter = mixFilterFunctions(filters[channelsInCurrentColorspace[i]].as<JsonArrayConst>());
    outputChannels[i] = globalFilter(filter(selectedPotentiometer < 6 ? fixedValues[selectedPotentiometer] : 0));
  }
  conversionFunction(outputChannels[0], outputChannels[1], outputChannels[2], filteredValues.red, filteredValues.green, filteredValues.blue);
  filteredValues.white = outputChannels[4];
  updateFilteredValues();
}


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
  knobMode = false;
  updateFilteredValues();
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
  server.on("/open_access_point", HTTP_GET, openAccessPointEndpoint);
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
  if (WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_AP)
    autoConnectWifi(); 
}
