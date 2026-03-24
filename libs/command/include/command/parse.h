#pragma once

#include <charconv>
#include <istream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

#include <core/type_traits.h>

#include <json/value.h>
#include <reflect_json/reflect_json.h>

#include "registry.h"
#include "text_parser.h"
#include "types.h"

namespace spie::command {

enum class input_format { text, json };

namespace details {

template <class T>
bool convertToken(std::string_view token, T &out, parse_storage &storage) {
  using U = std::remove_cv_t<std::remove_reference_t<T>>;

  if constexpr (std::is_same_v<U, std::string>) {
    out = std::string{token};
    return true;
  } else if constexpr (std::is_same_v<U, std::string_view>) {
    out = storage.own(token);
    return true;
  } else if constexpr (std::is_same_v<U, bool>) {
    if (token == "true" || token == "1") {
      out = true;
      return true;
    }
    if (token == "false" || token == "0") {
      out = false;
      return true;
    }
    return false;
  } else if constexpr (std::is_integral_v<U>) {
    U value{};
    auto [ptr, ec] =
        std::from_chars(token.data(), token.data() + token.size(), value);
    if (ec != std::errc{} || ptr != token.data() + token.size()) {
      return false;
    }
    out = value;
    return true;
  } else if constexpr (std::is_same_v<U, float>) {
    std::string str{token};
    std::size_t read = 0;
    try {
      out = std::stof(str, &read);
      return read == str.size();
    } catch (...) {
      return false;
    }
  } else if constexpr (std::is_same_v<U, double>) {
    std::string str{token};
    std::size_t read = 0;
    try {
      out = std::stod(str, &read);
      return read == str.size();
    } catch (...) {
      return false;
    }
  } else {
    return false;
  }
}

template <class Command>
bool bindPositionalArguments(const std::vector<std::string_view> &args,
               Command &command, parse_error &error,
               parse_storage &storage) {
  constexpr auto property_count =
      std::tuple_size<decltype(Command::properties)>::value;

  if (args.size() != property_count) {
    error.code = parse_error_code::argument_count_mismatch;
    error.message = "argument count mismatch";
    error.argument_index = args.size();
    return false;
  }

  bool success = true;
  mpl::for_sequence(std::make_index_sequence<property_count>{}, [&](auto i) {
    if (!success) {
      return;
    }

    constexpr auto prop = std::get<i>(Command::properties);
    using field_type = typename decltype(prop)::Type;

    if (!convertToken<field_type>(args[i], command.*(prop.member), storage)) {
      error.code = parse_error_code::conversion_error;
      error.message = "failed to convert positional argument";
      error.argument_index = i;
      success = false;
    }
  });

  return success;
}

template <class T>
bool assignFromJsonValue(const json::jsonValue &value, T &out,
                         parse_storage &storage);

template <class StructType, class Policy>
std::optional<StructType> fromJsonObject(const json::jsonObject &obj, Policy,
                                         parse_storage &storage) {
  StructType out{};
  bool ok = true;

  constexpr auto count = std::tuple_size_v<decltype(StructType::properties)>;
  mpl::for_sequence(std::make_index_sequence<count>{}, [&](auto i) {
    if (!ok) {
      return;
    }

    constexpr auto prop = std::get<i>(StructType::properties);
    const auto key = reflect::json_adapter::details::propertyKey(prop.name);
    const auto value = obj.getValue(key);
    using field_type = typename decltype(prop)::Type;

    if (!value.has_value()) {
      if constexpr (reflect::json_adapter::details::is_std_optional_v<field_type>) {
        out.*(prop.member) = std::nullopt;
        return;
      }

      if constexpr (Policy::fail_on_missing_field) {
        ok = false;
      }
      return;
    }

    if (!assignFromJsonValue<field_type>(*value, out.*(prop.member), storage)) {
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

template <class T>
bool assignFromJsonValue(const json::jsonValue &value, T &out,
                         parse_storage &storage) {
  using U = std::remove_cv_t<std::remove_reference_t<T>>;

  if constexpr (std::is_same_v<U, std::string_view>) {
    if (auto string_value = value.get<std::string>()) {
      out = storage.own(*string_value);
      return true;
    }
    return false;
  } else if constexpr (reflect::json_adapter::details::is_std_optional_v<U>) {
    using value_type = typename reflect::json_adapter::details::is_std_optional<U>::value_type;
    if (value.is<json::jsonNull>() || value.is<std::monostate>()) {
      out = std::nullopt;
      return true;
    }

    value_type parsed{};
    if (assignFromJsonValue(value, parsed, storage)) {
      out = std::move(parsed);
      return true;
    }
    return false;
  } else if constexpr (reflect::json_adapter::details::is_std_vector_v<U>) {
    using value_type = typename reflect::json_adapter::details::is_std_vector<U>::value_type;
    if (auto array = value.get<json::jsonArray>()) {
      U parsed;
      parsed.reserve(array->size());
      for (std::size_t index = 0; index < array->size(); ++index) {
        auto entry = array->getValue(static_cast<unsigned>(index));
        if (!entry.has_value()) {
          return false;
        }

        value_type element{};
        if (!assignFromJsonValue(*entry, element, storage)) {
          return false;
        }

        parsed.push_back(std::move(element));
      }
      out = std::move(parsed);
      return true;
    }
    return false;
  } else if constexpr (reflect::json_adapter::details::is_string_key_map_v<U>) {
    using mapped_type = typename reflect::json_adapter::details::is_string_key_map<U>::mapped_type;
    if (auto object = value.get<json::jsonObject>()) {
      U parsed;
      for (const auto &[key, nested] : *object) {
        mapped_type mapped{};
        if (!assignFromJsonValue(nested, mapped, storage)) {
          return false;
        }
        parsed.emplace(key, std::move(mapped));
      }
      out = std::move(parsed);
      return true;
    }
    return false;
  } else if constexpr (reflect::json_adapter::details::has_reflect_properties<U>::value) {
    if (auto object = value.get<json::jsonObject>()) {
      auto parsed = fromJsonObject<U, reflect::json_adapter::strict_policy>(
          *object, reflect::json_adapter::strict_policy{}, storage);
      if (!parsed.has_value()) {
        return false;
      }
      out = std::move(*parsed);
      return true;
    }
    return false;
  } else {
    return reflect::json_adapter::details::assignFromJsonValue<U>(value, out);
  }
}

template <class Command>
std::optional<Command> fromJsonObject(const json::jsonObject &obj, bool strict,
                                      parse_storage &storage) {
  if (strict) {
    return fromJsonObject<Command, reflect::json_adapter::strict_policy>(
        obj, reflect::json_adapter::strict_policy{}, storage);
  }

  return fromJsonObject<Command, reflect::json_adapter::permissive_policy>(
      obj, reflect::json_adapter::permissive_policy{}, storage);
}

inline std::optional<json::jsonValue> parseJsonValue(std::string_view input) {
  json::view view{input};
  json::value::Context context{view};
  auto machine =
      json::value::Machine(mpl::type_identity<json::value::States>{}, context);

  machine.emplace<json::value::Initial>();

  if (!machine.is_in<json::value::Finished>()) {
    return std::nullopt;
  }

  return context.getValue();
}

} // namespace details

template <class Registry>
parse_result<Registry> parse_text(std::string_view input) {
  parse_result<Registry> result{};

  result.storage->input = std::string{input};
  view text_view{result.storage->input_view()};
  text::Context context{text_view};
  auto machine = text::Machine(mpl::type_identity<text::States>{}, context);
  machine.emplace<text::Initial>();

  if (!machine.is_in<text::Finished>()) {
    result.error = context.error();
    if (result.error.code == parse_error_code::none) {
      result.error.code = parse_error_code::invalid_syntax;
      result.error.message = "text parser terminated in error state";
      result.error.position = text_view.pos_;
    }
    return result;
  }

  bool matched = false;
  Registry::for_each_binding([&](auto binding_identity) {
    using binding_type = typename decltype(binding_identity)::type;
    using command_type = typename binding_type::command_type;

    if (matched || context.commandName() != command_type::cmd) {
      return;
    }

    command_type command{};
    parse_error bind_error{};
    if (!details::bindPositionalArguments<command_type>(
          context.arguments(), command, bind_error, *result.storage)) {
      result.error = bind_error;
      matched = true;
      return;
    }

    result.command = command;
    matched = true;
  });

  if (!matched) {
    result.error.code = parse_error_code::unknown_command;
    result.error.message = "unknown command name";
    result.error.position = text_view.pos_;
  }

  return result;
}

template <class Registry>
parse_result<Registry> parse_json(std::string_view input, bool strict = true) {
  parse_result<Registry> result{};

  result.storage->input = std::string{input};
  auto parsed = details::parseJsonValue(result.storage->input_view());
  if (!parsed.has_value()) {
    result.error.code = parse_error_code::json_error;
    result.error.message = "failed to parse json payload";
    return result;
  }

  if (!parsed->template is<json::jsonObject>()) {
    result.error.code = parse_error_code::json_shape_error;
    result.error.message = "json payload must be an object";
    return result;
  }

  auto root = parsed->template get<json::jsonObject>();
  if (!root.has_value()) {
    result.error.code = parse_error_code::json_shape_error;
    result.error.message = "json payload object extraction failed";
    return result;
  }

  json::jsonObject payload = *root;
  std::optional<std::string> requested_command{};

  if (auto cmd_value = root->getValue("cmd"); cmd_value.has_value()) {
    auto command_string = cmd_value->template get<std::string>();
    if (!command_string.has_value()) {
      result.error.code = parse_error_code::json_shape_error;
      result.error.message = "field 'cmd' must be a string";
      return result;
    }
    requested_command = *command_string;

    if (auto payload_value = root->getValue("payload"); payload_value.has_value()) {
      auto payload_object = payload_value->template get<json::jsonObject>();
      if (!payload_object.has_value()) {
        result.error.code = parse_error_code::json_shape_error;
        result.error.message = "field 'payload' must be an object";
        return result;
      }
      payload = *payload_object;
    }
  }

  bool command_known = false;
  std::size_t matches = 0;

  Registry::for_each_binding([&](auto binding_identity) {
    using binding_type = typename decltype(binding_identity)::type;
    using command_type = typename binding_type::command_type;

    if (requested_command.has_value() &&
        command_type::cmd == std::string_view{*requested_command}) {
      command_known = true;
    }

    if (requested_command.has_value() &&
        command_type::cmd != std::string_view{*requested_command}) {
      return;
    }

    auto maybe_command =
        details::fromJsonObject<command_type>(payload, strict, *result.storage);
    if (!maybe_command.has_value()) {
      return;
    }

    ++matches;
    if (matches == 1U) {
      result.command = *maybe_command;
    }
  });

  if (matches == 1U) {
    return result;
  }

  if (matches > 1U) {
    result.error.code = parse_error_code::ambiguous_command;
    result.error.message = "json payload matches multiple command types";
    return result;
  }

  if (requested_command.has_value() && !command_known) {
    result.error.code = parse_error_code::unknown_command;
    result.error.message = "requested json command is not registered";
    return result;
  }

  result.error.code = parse_error_code::json_shape_error;
  result.error.message = "json payload does not match any command schema";
  return result;
}

template <class Registry>
parse_result<Registry> parse(std::string_view input,
                             input_format format = input_format::text,
                             bool strict_json = true) {
  if (format == input_format::json) {
    return parse_json<Registry>(input, strict_json);
  }

  return parse_text<Registry>(input);
}

template <class Registry>
parse_result<Registry> parse(std::istream &input,
                             input_format format = input_format::text,
                             bool strict_json = true) {
  std::string buffer(std::istreambuf_iterator<char>{input}, {});
  return parse<Registry>(std::string_view{buffer}, format, strict_json);
}

} // namespace spie::command
