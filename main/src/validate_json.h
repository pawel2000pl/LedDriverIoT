#pragma once
#include "lib/ArduinoJson/ArduinoJson.h"

#ifndef ARDUINO
  using String = std::string;
#endif
 
extern JsonDocument JsonEmpty;
extern JsonDocument JsonNull;

String validateJson(const JsonVariant& object, const JsonVariantConst& schema, const JsonVariantConst& objectType, String path=".", const JsonVariantConst& defaults=JsonNull);
