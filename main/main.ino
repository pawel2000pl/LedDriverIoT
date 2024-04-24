#include <WiFi.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include "json.h"
#include "utils.h"
#include "resources.h"

const char* ssid = "iot_test";
const char* password = "password123";

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,254);
IPAddress subnet(255,255,255,0);

WebServer server(80);

void setup() {
  Serial.begin(115200);
  
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.println("Working as access-point");
  Serial.println(IP);

  server.on("/", HTTP_GET, [&server](){server.send(200, "text/html", "<meta http-equiv=\"refresh\" content=\"0; url=/index.html\">");});
  for (unsigned i=0;i<resources_count;i++) 
    server.on(resources[i]->name, HTTP_GET, [i, &server](){sendDecompressedData(server, resources[i]->mime_type, resources[i]->data, resources[i]->size, resources[i]->decompressed_size);});
 
  server.begin();
}

void loop() {
  server.handleClient();
}
