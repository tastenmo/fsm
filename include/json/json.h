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

using jsonKeyValuePair = std::pair<std::string, jsonValue>;

class jsonObject {

public:
  void addValue(jsonKeyValuePair);

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

} // namespace escad::json
