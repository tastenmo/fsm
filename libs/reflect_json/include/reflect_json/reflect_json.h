#pragma once

#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <concepts>
#include <map>
#include <vector>
#include <json/json.h>
#include <reflect/property.h>

namespace spie::reflect::json_adapter {

struct strict_policy {
  static constexpr bool fail_on_missing_field = true;
  static constexpr bool fail_on_type_mismatch = true;
};

struct permissive_policy {
  static constexpr bool fail_on_missing_field = false;
  static constexpr bool fail_on_type_mismatch = false;
};

template <typename StructType, typename Policy>
std::optional<StructType> fromJsonObject(const json::jsonObject &obj, Policy);

template <typename StructType>
std::optional<StructType> fromJsonObject(const json::jsonObject &obj);

namespace details {

template <typename T, typename = void>
struct has_reflect_properties : std::false_type {};

template <typename T>
struct has_reflect_properties<
    T, std::void_t<decltype(std::tuple_size<decltype(T::properties)>::value)>>
    : std::true_type {};

template <typename T> struct is_std_optional : std::false_type {};

template <typename T> struct is_std_optional<std::optional<T>> : std::true_type {
  using value_type = T;
};

template <typename T>
inline constexpr bool is_std_optional_v = is_std_optional<T>::value;

template <typename T> struct is_std_vector : std::false_type {};

template <typename T, typename Allocator>
struct is_std_vector<std::vector<T, Allocator>> : std::true_type {
  using value_type = T;
};

template <typename T>
inline constexpr bool is_std_vector_v = is_std_vector<T>::value;

template <typename T> struct is_string_key_map : std::false_type {};

template <typename Value, typename Compare, typename Allocator>
struct is_string_key_map<std::map<std::string, Value, Compare, Allocator>>
    : std::true_type {
  using mapped_type = Value;
};

template <typename T>
inline constexpr bool is_string_key_map_v = is_string_key_map<T>::value;

template <typename Name>
std::string fromPointerName(Name name) {
  return std::string{name == nullptr ? "" : name};
}

template <typename Name> std::string propertyKey(const Name &name) {
  if constexpr (std::is_convertible_v<Name, std::string_view>) {
    return std::string{std::string_view{name}};
  } else if constexpr (std::is_convertible_v<Name,
                                              std::basic_string_view<char32_t>>) {
    const auto view = std::basic_string_view<char32_t>{name};
    std::string out;
    out.reserve(view.size());
    for (char32_t ch : view) {
      out.push_back(static_cast<char>(ch));
    }
    return out;
  } else if constexpr (std::is_pointer_v<Name> &&
                       std::is_same_v<
                           std::remove_cv_t<std::remove_pointer_t<Name>>, char>) {
    return fromPointerName(name);
  } else if constexpr (requires(const Name &n) {
                         { n.data() };
                         { n.size() } -> std::convertible_to<std::size_t>;
                       }) {
    std::size_t len = static_cast<std::size_t>(name.size());
    if (len > 0 && name.data()[len - 1] == '\0') {
      --len;
    }
    return std::string{name.data(), len};
  } else {
    static_assert(!sizeof(Name), "property name type must convert to string_view");
  }
}

template <typename T> json::jsonValue toJsonValue(const T &value) {
  using U = std::remove_cv_t<std::remove_reference_t<T>>;

  if constexpr (is_std_optional_v<U>) {
    if (value.has_value()) {
      return toJsonValue(*value);
    }
    return json::jsonValue{json::jsonNull{}};
  } else if constexpr (is_std_vector_v<U>) {
    json::jsonArray array;
    for (const auto &entry : value) {
      array.addValue(toJsonValue(entry));
    }
    return json::jsonValue{array};
  } else if constexpr (is_string_key_map_v<U>) {
    json::jsonObject object;
    for (const auto &[key, entry] : value) {
      object.addValue({key, toJsonValue(entry)});
    }
    return json::jsonValue{object};
  } else if constexpr (has_reflect_properties<U>::value) {
    json::jsonObject nested;
    constexpr auto count = std::tuple_size<decltype(U::properties)>::value;
    mpl::for_sequence(std::make_index_sequence<count>{}, [&](auto i) {
      constexpr auto prop = std::get<i>(U::properties);
      nested.addValue(
          {propertyKey(prop.name), toJsonValue(value.*(prop.member))});
    });
    return json::jsonValue{nested};
  } else if constexpr (std::is_same_v<U, std::string>) {
    return json::jsonValue{value};
  } else if constexpr (std::is_same_v<U, std::string_view>) {
    return json::jsonValue{std::string{value}};
  } else if constexpr (std::is_same_v<U, const char *>) {
    return json::jsonValue{std::string{value}};
  } else if constexpr (std::is_same_v<U, bool>) {
    return json::jsonValue{value};
  } else if constexpr (std::is_floating_point_v<U>) {
    return json::jsonValue{json::number::JsonNumber{static_cast<double>(value)}};
  } else if constexpr (std::is_integral_v<U> && std::is_signed_v<U>) {
    if constexpr (sizeof(U) <= sizeof(int)) {
      return json::jsonValue{json::number::JsonNumber{static_cast<int>(value)}};
    } else {
      return json::jsonValue{json::number::JsonNumber{static_cast<int64_t>(value)}};
    }
  } else if constexpr (std::is_integral_v<U>) {
    if constexpr (sizeof(U) <= sizeof(unsigned)) {
      return json::jsonValue{json::number::JsonNumber{static_cast<unsigned>(value)}};
    } else {
      return json::jsonValue{json::number::JsonNumber{static_cast<uint64_t>(value)}};
    }
  } else {
    static_assert(!sizeof(U), "unsupported field type for json conversion");
  }
}

template <typename T>
bool assignFromJsonNumber(const json::number::JsonNumber &num, T &out) {
  if constexpr (std::is_floating_point_v<T>) {
    if (auto v = num.get<double>()) {
      out = static_cast<T>(*v);
      return true;
    }
    if (auto v = num.get<int64_t>()) {
      out = static_cast<T>(*v);
      return true;
    }
    if (auto v = num.get<uint64_t>()) {
      out = static_cast<T>(*v);
      return true;
    }
    if (auto v = num.get<int>()) {
      out = static_cast<T>(*v);
      return true;
    }
    if (auto v = num.get<unsigned>()) {
      out = static_cast<T>(*v);
      return true;
    }
    return false;
  } else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
    if (auto v = num.get<int>()) {
      out = static_cast<T>(*v);
      return true;
    }
    if (auto v = num.get<int64_t>()) {
      if (*v < static_cast<int64_t>(std::numeric_limits<T>::min()) ||
          *v > static_cast<int64_t>(std::numeric_limits<T>::max())) {
        return false;
      }
      out = static_cast<T>(*v);
      return true;
    }
    if (auto v = num.get<unsigned>()) {
      if (*v > static_cast<unsigned>(std::numeric_limits<T>::max())) {
        return false;
      }
      out = static_cast<T>(*v);
      return true;
    }
    if (auto v = num.get<uint64_t>()) {
      if (*v > static_cast<uint64_t>(std::numeric_limits<T>::max())) {
        return false;
      }
      out = static_cast<T>(*v);
      return true;
    }
    if (auto v = num.get<double>()) {
      out = static_cast<T>(*v);
      return true;
    }
    return false;
  } else if constexpr (std::is_integral_v<T>) {
    if (auto v = num.get<unsigned>()) {
      out = static_cast<T>(*v);
      return true;
    }
    if (auto v = num.get<uint64_t>()) {
      if (*v > static_cast<uint64_t>(std::numeric_limits<T>::max())) {
        return false;
      }
      out = static_cast<T>(*v);
      return true;
    }
    if (auto v = num.get<int>()) {
      if (*v < 0 || static_cast<uint64_t>(*v) >
                        static_cast<uint64_t>(std::numeric_limits<T>::max())) {
        return false;
      }
      out = static_cast<T>(*v);
      return true;
    }
    if (auto v = num.get<int64_t>()) {
      if (*v < 0 || static_cast<uint64_t>(*v) >
                        static_cast<uint64_t>(std::numeric_limits<T>::max())) {
        return false;
      }
      out = static_cast<T>(*v);
      return true;
    }
    if (auto v = num.get<double>()) {
      if (*v < 0) {
        return false;
      }
      out = static_cast<T>(*v);
      return true;
    }
    return false;
  } else {
    return false;
  }
}

template <typename T> bool assignFromJsonValue(const json::jsonValue &value, T &out) {
  using U = std::remove_cv_t<std::remove_reference_t<T>>;

  if constexpr (is_std_optional_v<U>) {
    using ValueType = typename is_std_optional<U>::value_type;
    if (value.is<json::jsonNull>() || value.is<std::monostate>()) {
      out = std::nullopt;
      return true;
    }
    ValueType parsed{};
    if (assignFromJsonValue<ValueType>(value, parsed)) {
      out = std::move(parsed);
      return true;
    }
    return false;
  } else if constexpr (is_std_vector_v<U>) {
    using ValueType = typename is_std_vector<U>::value_type;
    if (auto array = value.get<json::jsonArray>()) {
      U parsed;
      parsed.reserve(array->size());
      for (std::size_t index = 0; index < array->size(); ++index) {
        auto entry = array->getValue(static_cast<unsigned>(index));
        if (!entry.has_value()) {
          return false;
        }

        ValueType element{};
        if (!assignFromJsonValue<ValueType>(*entry, element)) {
          return false;
        }

        parsed.push_back(std::move(element));
      }

      out = std::move(parsed);
      return true;
    }
    return false;
  } else if constexpr (is_string_key_map_v<U>) {
    using MappedType = typename is_string_key_map<U>::mapped_type;
    if (auto object = value.get<json::jsonObject>()) {
      U parsed;
      for (const auto &[key, entryValue] : *object) {
        MappedType mapped{};
        if (!assignFromJsonValue<MappedType>(entryValue, mapped)) {
          return false;
        }
        parsed.emplace(key, std::move(mapped));
      }

      out = std::move(parsed);
      return true;
    }
    return false;
  } else if constexpr (has_reflect_properties<U>::value) {
    if (auto nested = value.get<json::jsonObject>()) {
      auto parsed = ::spie::reflect::json_adapter::fromJsonObject<U>(*nested);
      if (parsed.has_value()) {
        out = std::move(*parsed);
        return true;
      }
    }
    return false;
  } else if constexpr (std::is_same_v<U, std::string>) {
    if (auto v = value.get<std::string>()) {
      out = *v;
      return true;
    }
    return false;
  } else if constexpr (std::is_same_v<U, bool>) {
    if (auto v = value.get<bool>()) {
      out = *v;
      return true;
    }
    return false;
  } else if constexpr (std::is_arithmetic_v<U>) {
    if (auto number = value.get<json::number::JsonNumber>()) {
      return assignFromJsonNumber(*number, out);
    }
    return false;
  } else {
    static_assert(!sizeof(U), "unsupported field type for json conversion");
  }
}

} // namespace details

template <typename StructType> json::jsonObject toJsonObject(const StructType &object) {
  json::jsonObject obj;

  constexpr auto count = std::tuple_size<decltype(StructType::properties)>::value;
  mpl::for_sequence(std::make_index_sequence<count>{}, [&](auto i) {
    constexpr auto prop = std::get<i>(StructType::properties);
    obj.addValue({details::propertyKey(prop.name),
                  details::toJsonValue(object.*(prop.member))});
  });

  return obj;
}

template <typename StructType, typename Policy>
std::optional<StructType> fromJsonObject(const json::jsonObject &obj,
                                         Policy) {
  static_assert(std::is_same_v<decltype(Policy::fail_on_missing_field),
                               const bool>);
  static_assert(std::is_same_v<decltype(Policy::fail_on_type_mismatch),
                               const bool>);

  StructType out{};
  bool ok = true;

  constexpr auto count = std::tuple_size<decltype(StructType::properties)>::value;
  mpl::for_sequence(std::make_index_sequence<count>{}, [&](auto i) {
    if (!ok) {
      return;
    }

    constexpr auto prop = std::get<i>(StructType::properties);
    const auto key = details::propertyKey(prop.name);
    const auto value = obj.getValue(key);
    if (!value.has_value()) {
      using FieldType = typename decltype(prop)::Type;
      if constexpr (details::is_std_optional_v<FieldType>) {
        out.*(prop.member) = std::nullopt;
        return;
      }
      if constexpr (Policy::fail_on_missing_field) {
        ok = false;
      }
      return;
    }

    using FieldType = typename decltype(prop)::Type;
    if (!details::assignFromJsonValue<FieldType>(*value, out.*(prop.member))) {
      if constexpr (Policy::fail_on_type_mismatch) {
        ok = false;
      }
    }
  });

  if (!ok) {
    return std::nullopt;
  }

  return out;
}

template <typename StructType>
std::optional<StructType> fromJsonObject(const json::jsonObject &obj) {
  return fromJsonObject<StructType, strict_policy>(obj, strict_policy{});
}

} // namespace spie::reflect::json_adapter
