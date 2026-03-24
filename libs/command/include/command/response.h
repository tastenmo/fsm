#pragma once

#include <sstream>
#include <string>
#include <type_traits>

#include <reflect_json/reflect_json.h>

#include "dispatcher.h"

namespace spie::command {

namespace details {

template <typename T, typename = void>
struct has_reflect_properties : std::false_type {};

template <typename T>
struct has_reflect_properties<
    T, std::void_t<decltype(std::tuple_size<decltype(T::properties)>::value)>>
    : std::true_type {};

template <typename T> std::string reflectedToText(const T &value) {
  std::ostringstream stream;
  bool first = true;

  constexpr auto count = std::tuple_size_v<decltype(T::properties)>;
  mpl::for_sequence(std::make_index_sequence<count>{}, [&](auto i) {
    constexpr auto prop = std::get<i>(T::properties);
    if (!first) {
      stream << ' ';
    }
    first = false;
    stream << reflect::json_adapter::details::propertyKey(prop.name) << '=';
    stream << value.*(prop.member);
  });

  return stream.str();
}

template <typename T> json::jsonValue reflectedToJsonValue(const T &value) {
  if constexpr (has_reflect_properties<T>::value) {
    return json::jsonValue{reflect::json_adapter::toJsonObject(value)};
  } else {
    return reflect::json_adapter::details::toJsonValue(value);
  }
}

} // namespace details

inline json::jsonObject toJsonObject(const parse_error &error) {
  json::jsonObject obj;
  obj.addValue({"code", json::jsonValue{std::string{error.message}}});
  obj.addValue({"position",
                json::jsonValue{json::number::JsonNumber{
                    static_cast<uint64_t>(error.position)}}});
  obj.addValue({"argumentIndex",
                json::jsonValue{json::number::JsonNumber{
                    static_cast<uint64_t>(error.argument_index)}}});
  return obj;
}

inline std::string toText(const parse_error &error) {
  std::ostringstream stream;
  stream << "parse_error{" << error.message << ", position=" << error.position
         << ", argument=" << error.argument_index << '}';
  return stream.str();
}

inline json::jsonObject toJsonObject(const dispatch_error &error) {
  json::jsonObject obj;
  obj.addValue({"message", json::jsonValue{error.message}});
  return obj;
}

inline std::string toText(const dispatch_error &error) {
  return "dispatch_error{" + error.message + '}';
}

template <class Registry>
json::jsonObject toJsonObject(const parse_result<Registry> &result) {
  json::jsonObject obj;

  if (!result.ok()) {
    obj.addValue({"ok", json::jsonValue{false}});
    obj.addValue({"error", json::jsonValue{toJsonObject(result.error)}});
    return obj;
  }

  obj.addValue({"ok", json::jsonValue{true}});
  std::visit(
      overloaded{[](const std::monostate &) {},
                 [&](const auto &typed_command) {
                   obj.addValue(
                       {"command", details::reflectedToJsonValue(typed_command)});
                 }},
      result.command);

  return obj;
}

template <class Registry> std::string toText(const parse_result<Registry> &result) {
  if (!result.ok()) {
    return toText(result.error);
  }

  std::string rendered{"command{"};
  std::visit(
      overloaded{[](const std::monostate &) {},
                 [&](const auto &typed_command) {
                   rendered += details::reflectedToText(typed_command);
                 }},
      result.command);
  rendered += '}';
  return rendered;
}

template <class Registry>
json::jsonObject toJsonObject(const dispatch_result<Registry> &result) {
  json::jsonObject obj;

  if (!result.ok()) {
    obj.addValue({"ok", json::jsonValue{false}});
    obj.addValue({"error", json::jsonValue{toJsonObject(*result.error)}});
    return obj;
  }

  obj.addValue({"ok", json::jsonValue{true}});
  std::visit(
      overloaded{[](const std::monostate &) {},
                 [&](const auto &typed_response) {
                   obj.addValue(
                       {"response", details::reflectedToJsonValue(typed_response)});
                 }},
      result.response);

  return obj;
}

template <class Registry>
std::string toText(const dispatch_result<Registry> &result) {
  if (!result.ok()) {
    return toText(*result.error);
  }

  std::string rendered{"response{"};
  std::visit(
      overloaded{[](const std::monostate &) {},
                 [&](const auto &typed_response) {
                   rendered += details::reflectedToText(typed_response);
                 }},
      result.response);
  rendered += '}';
  return rendered;
}

} // namespace spie::command