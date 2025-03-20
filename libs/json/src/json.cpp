
#include <json/json.h>

namespace escad::json {

void jsonObject::addValue(jsonKeyValuePair val) { values_.emplace(val); }

jsonValue jsonObject::getValue(std::string_view key) {
  return values_[std::string(key)];
}

void jsonArray::addValue(jsonValue val) { values_.push_back(val); }

jsonValue jsonArray::getValue(unsigned index) { return values_[index]; }

} // namespace escad::json