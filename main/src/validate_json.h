#pragma once
#include <ArduinoJson.h>

#ifndef ARDUINO
  using String = std::string;
#endif
 
extern StaticJsonDocument<64> JsonEmpty;
extern StaticJsonDocument<64> JsonNull;

String validateJson(JsonVariant object, const JsonVariantConst& schema, const JsonVariantConst& objectType, String path=".", const JsonVariantConst& defaults=JsonNull);
