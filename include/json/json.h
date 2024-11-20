#pragma once

#include <cmath>

#include <map>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <vector>

#include "number.h"
#include "string.h"

#include <magic_enum/magic_enum.hpp>

using namespace escad::new_fsm;

namespace escad::json {

class jsonObject;
class jsonArray;

using jsonValue = std::variant<std::monostate, bool, std::string,
                               number::JsonNumber, jsonObject, jsonArray>;

class jsonObject {

public:
  void addValue(std::string key, jsonValue val);

  jsonValue getValue(std::string_view key);

private:
  std::map<std::string, jsonValue> values_;
};

class jsonArray {
public:
  void addValue(jsonValue val);

private:
  std::vector<jsonValue> values_;
};

void jsonObject::addValue(std::string key, jsonValue val) {
  values_[key] = val;
}

jsonValue jsonObject::getValue(std::string_view key) {
  return values_[std::string(key)];
}

void jsonArray::addValue(jsonValue val) { values_.push_back(val); }

} // namespace escad::json
