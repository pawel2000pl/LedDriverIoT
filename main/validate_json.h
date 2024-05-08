#pragma once
#include "json.h"

#ifndef ARDUINO
  using String = std::string;
#endif

String validateJson(const JsonVariantConst& object, const JsonVariantConst& schema, const JsonVariantConst& objectType, String path=".");
String validateJson(const JsonVariantConst& object, const JsonVariantConst& schema);
