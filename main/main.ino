#include <WiFi.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include <Preferences.h>
#include "json.h"
#include "utils.h"
#include "resources.h"

#define CONFIGURATION_FILENAME "configuration.json"
#define JSON_CONFIG_BUF_SIZE (16*1024)
Preferences preferences;
StaticJsonDocument<JSON_CONFIG_BUF_SIZE> configuration;

void loadConfiguration() {
  uint8_t decompressed_buffer[default_config_json_decompressed_size+1];
  size_t decompressed_size = fastlz_decompress(default_config_json_data, default_config_json_size, decompressed_buffer, default_config_json_decompressed_size);
  decompressed_buffer[decompressed_size] = 0;
  String buf = preferences.getString(CONFIGURATION_FILENAME, (const char*)decompressed_buffer);
  DeserializationError err = deserializeJson(configuration, buf.c_str());
  if (err != DeserializationError::Ok)
    deserializeJson(configuration, (const char*)decompressed_buffer);
}

const char* ssid = "iot_test";
const char* password = "password123";

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,254);
IPAddress subnet(255,255,255,0);

WebServer server(80);

void autoConnectWifi() {
  const auto& wifiConfig = configuration["wifi"];
  const auto& staPriority = wifiConfig["sta_priority"];
  const auto& apConfig = wifiConfig["access_point"];
  for (int i=0;i<staPriority.size();i++) {
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to ");
    Serial.println(staPriority[i]["ssid"].as<String>());
    if (WiFi.begin(staPriority[i]["ssid"].as<String>(), staPriority[i]["password"].as<String>()) == WL_CONNECTED)
      return;
  }
  if (apConfig["enabled"]) {
    Serial.println("Switching to AP-mode");
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_ip, gateway, subnet);
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


void setup() {
  Serial.begin(115200);

  preferences.begin("led-driver", false);
  loadConfiguration();
  autoConnectWifi();

  server.on("/", HTTP_GET, [&server](){server.send(200, "text/html", "<meta http-equiv=\"refresh\" content=\"0; url=/index.html\">");});
  server.on("/config.json", HTTP_GET, sendConfiguration);
  server.on("/favicon.ico", HTTP_GET, [&server](){sendDecompressedData(server, resource_favicon_svg.mime_type, resource_favicon_svg.data, resource_favicon_svg.size, resource_favicon_svg.decompressed_size);});
  for (unsigned i=0;i<resources_count;i++)
    server.on(resources[i]->name, HTTP_GET, [i, &server](){sendDecompressedData(server, resources[i]->mime_type, resources[i]->data, resources[i]->size, resources[i]->decompressed_size);});
 
  server.begin();

}

void loop() {
  server.handleClient();
}
