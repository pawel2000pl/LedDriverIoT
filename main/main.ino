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
#include "color_endpoints.h"
#include "ledc_driver.h"

#define CONNECTION_TIMEOUT 15000
#define CONFIGURATION_FILENAME "/configuration.json"
#define JSON_CONFIG_BUF_SIZE (16*1024)
#define MAX_STRING_LENGTH 64

const int LED_GPIO_OUTPUTS[] = {D7, D8, D9, D10};
const int POTENTIOMETER_GPIO_INPUTS[] = {A0, A1, A2, A3};
const int RESET_CONFIGURATION_PIN = D4;

StaticJsonDocument<JSON_CONFIG_BUF_SIZE> configuration;
StaticJsonDocument<JSON_CONFIG_BUF_SIZE> configSchema;
StaticJsonDocument<JSON_CONFIG_BUF_SIZE> scannedNetworks;
std::list<std::function<void()>> taskQueue;

WebServer server(80);

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


int sendDecompressedData(WebServer& server, const char* content_type, const void* compressed_buffer, size_t compressed_size, size_t max_decompressed_size) {
    uint8_t* decompressed_buffer = new uint8_t[max_decompressed_size+1];
    size_t decompressed_size = fastlz_decompress(compressed_buffer, compressed_size, decompressed_buffer, max_decompressed_size);
    if (decompressed_size == 0) {
        sendError("Decompression error");
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
  else if (WiFi.getMode() == WIFI_AP)
    WiFi.softAPdisconnect();
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
  WiFi.softAP(apConfig["ssid"].as<String>(), apConfig["password"].as<String>(), 1, apConfig["hidden"].as<bool>(), 16);
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
ColorChannels lastKnobValues = {0,0,0,0};
ColorChannels outputValues = {0,0,0,0};
bool knobMode = true;
bool outputRequiresUpdate = true;


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
  for (int i=0;i<4;i++) 
    setLedC(LED_GPIO_OUTPUTS[i], i, outputValues[i]);
}


void updateFilteredValues() {
  filteredValues = (RGBW)somethingColorCustom(
      configuration, rgbToRgb, "globalOutputFilters", "outputFilters", NULL,
      false, false,
      "red", "green", "blue",
      colorValues.red, colorValues.green, colorValues.blue, colorValues.white
    );
}


void updateColorValues(float r, float g, float b, float w) {
  colorValues = RGBW(r, g, b, w);
  outputRequiresUpdate = true;
}


void setFromKnobs(const ColorChannels values) {
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
  const auto applyBias = [biasUp, biasDown](float x) { return constrain((x - biasDown) / (1.f - biasUp - biasDown), 0, 1); };
  const float fixedValues[] = {applyBias(values[0]), applyBias(values[1]), applyBias(values[2]), applyBias(values[3]), 0, 1};
  const String knobColorspace = configuration["channels"]["knobMode"];
  const char* colorspaces[] = {"hsv", "hsl", "rgb"};
  const char* channels[][4] = {{"hue", "saturation", "value", "white"}, {"hue", "saturation", "lightness", "white"}, {"red", "green", "blue", "white"}};
  const char** channelsInCurrentColorspace = channels[0];
  for (int i=0;i<3;i++)
    if (knobColorspace == colorspaces[i])
      channelsInCurrentColorspace = channels[i];
  const auto& potentionemterConfiguration = configuration["hardware"]["potentionemterConfiguration"];
  float outputChannels[4];
  for (int i=0;i<4;i++) {
    unsigned selectedPotentiometer = (unsigned)potentionemterConfiguration[channelsInCurrentColorspace[i]].as<int>();
    outputChannels[i] = selectedPotentiometer < 6 ? fixedValues[selectedPotentiometer] : 0;
  }
  ColorChannels filteredChannels = setColorAuto(configuration, knobColorspace, outputChannels[0], outputChannels[1], outputChannels[2], outputChannels[3]);
  updateColorValues(filteredChannels[0], filteredChannels[1], filteredChannels[2], filteredChannels[3]);
}

void checkKnobs() {
  ColorChannels values;
  for (int i=0;i<4;i++) {
    double readed = analogRead(POTENTIOMETER_GPIO_INPUTS[i]);
    values[i] = readed / 4095;
  }
  setFromKnobs(values);
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
  outputRequiresUpdate = true;
  sendColors();
}


void sendColorsAuto() {
  ColorChannels channels = getColorAuto(configuration, configuration["channels"]["webMode"], 
    colorValues.red, colorValues.green, colorValues.blue, colorValues.white
  );
  StaticJsonDocument<256> data;
  data.add(channels[0]);
  data.add(channels[1]);
  data.add(channels[2]);
  data.add(channels[3]);
  char buf[256];
  int size = serializeJson(data, buf, 255);
  buf[size] = 0;
  server.send(200, default_config_json_mime_type, buf);
}


void setColorsAuto() {
  StaticJsonDocument<256> data;
  String rawData = server.arg("plain");
  DeserializationError err = deserializeJson(data, rawData);
  if (err != DeserializationError::Ok) 
    return sendError("Deserialization error");
  String assertMessage = validateJson(data, configSchema, configSchema["color-channels"]);
  if (assertMessage != "")
    sendError(assertMessage, 422);
  ColorChannels channels = setColorAuto(
    configuration, configuration["channels"]["webMode"], 
    data[0].as<float>(), data[1].as<float>(), data[2].as<float>(), data[3].as<float>()
  );
  knobMode = false;
  updateColorValues(channels[0], channels[1], channels[2], channels[3]);
  sendOk();   
}

int floatFilter15(float x) { 
  return (int)round(x * 15.f); 
};

void simpleMode() {
  if (server.method() == HTTP_POST) {
    auto fun = [](String value) { return constrain((double)atoi(value.c_str())/15.f, 0, 1);};    
    ColorChannels channels = setColorAuto(configuration, configuration["channels"]["webMode"], 
      fun(server.arg("ch0")), fun(server.arg("ch1")), fun(server.arg("ch2")), fun(server.arg("ch3")));
    knobMode = false;
    updateColorValues(channels[0], channels[1], channels[2], channels[3]);
  }
  uint8_t* decompressed_buffer = new uint8_t[simple_template_html_decompressed_size+1];
  char* render_buffer = new char[simple_template_html_decompressed_size+256];
  size_t decompressed_size = fastlz_decompress(simple_template_html_data, simple_template_html_size, decompressed_buffer, simple_template_html_decompressed_size);
  if (decompressed_size == 0) {
      delete [] render_buffer;
      delete [] decompressed_buffer;
      sendError("Decompression error");
  }
  decompressed_buffer[decompressed_size] = 0;
  const char* colorspaces[] = {"hsv", "hsl", "rgb"};
  const char* channels[][4] = {{"hue", "saturation", "value", "white"}, {"hue", "saturation", "lightness", "white"}, {"red", "green", "blue", "white"}};
  const char** channelsInCurrentColorspace = channels[0];
  String destColorspace = configuration["channels"]["webMode"];
  for (int i=0;i<3;i++)
    if (destColorspace == colorspaces[i])
      channelsInCurrentColorspace = channels[i];
  ColorChannels filteredChannels = getColorAuto(configuration, configuration["channels"]["webMode"], 
    colorValues.red, colorValues.green, colorValues.blue, colorValues.white
  );
  sprintf(
    render_buffer, (const char*)decompressed_buffer, 
    channelsInCurrentColorspace[0],
    floatFilter15(filteredChannels[0]),
    channelsInCurrentColorspace[1],
    floatFilter15(filteredChannels[1]),
    channelsInCurrentColorspace[2],
    floatFilter15(filteredChannels[2]),
    configuration["hardware"]["enbleWhiteKnob"].as<bool>() ? "" : "style=\"display: none;\"",
    channelsInCurrentColorspace[3],
    floatFilter15(filteredChannels[3])
  );
  server.send(200, "text/html", (const char*)render_buffer);
  delete [] render_buffer;
  delete [] decompressed_buffer;
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
  server.on("/filtered_color.json", HTTP_GET, sendColorsAuto);
  server.on("/filtered_color.json", HTTP_POST, setColorsAuto);
  server.on("/simple.html", HTTP_GET, simpleMode);
  server.on("/simple.html", HTTP_POST, simpleMode);
  server.begin();
}


unsigned long long int reset_timer = 0;


void setup() {
  Serial.begin(115200);
  
  pinMode(RESET_CONFIGURATION_PIN, INPUT_PULLUP);
  analogReadResolution(12);
  initLedC();
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

  checkKnobs();

  if (outputRequiresUpdate) {
    updateFilteredValues();
    updateOutputs();
  }

  if (digitalRead(RESET_CONFIGURATION_PIN) == 0) {
    if (reset_timer == 0)
      reset_timer = millis();
    else if (millis() - reset_timer >= 10000) {
      SPIFFS.remove(CONFIGURATION_FILENAME);
      ESP.restart();
    }      
  } else 
    reset_timer = 0;
}
