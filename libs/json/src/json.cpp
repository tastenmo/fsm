
#include <json/json.h>

namespace spie::json {

void jsonObject::addValue(jsonKeyValuePair val) { values_.emplace(val); }

std::optional<jsonValue> jsonObject::getValue(std::string_view key) const {
   auto it = values_.find(std::string(key));
   if (it != values_.end()) {
      return it->second;
   }
   return std::nullopt;
}

std::string jsonObject::toString() const {
   std::string result = "{";
   for (auto it = values_.begin(); it != values_.end(); ++it) {
      result += "\"" + it->first + "\": " + it->second.toString();
      if (std::next(it) != values_.end()) {
         result += ", ";
      }
   }
   result += "}";
   return result;
}

void jsonArray::addValue(jsonValue val) { values_.push_back(val); }

std::optional<jsonValue> jsonArray::getValue(unsigned index) const {
   if (index < values_.size()) {
      return values_[index];
   }
   return std::nullopt;
}

std::string jsonArray::toString() const {
   std::string result = "[";
   for (size_t i = 0; i < values_.size(); ++i) {
      result += values_[i].toString();
      if (i < values_.size() - 1) {
         result += ", ";
      }
   }
   result += "]";
   return result;
}

} // namespace spie::json