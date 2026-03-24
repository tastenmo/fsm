#include <sstream>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

#include <command/dispatcher.h>
#include <command/parse.h>
#include <command/response.h>
#include <command/schema.h>
#include <reflect/property.h>

namespace refl = spie::reflect;

struct cfarCfg {
  int subFrameIdx;
  int direction;
  int mode;
  int noiseWindow;
  int guardLength;
  int shiftDivisor;
  int cyclic;
  int threshold;
  int peakGrouping;

  constexpr static std::string_view cmd{"cfarCfg"};

  constexpr static auto properties = std::make_tuple(
      refl::property(&cfarCfg::subFrameIdx, "subFrameIdx"),
      refl::property(&cfarCfg::direction, "direction"),
      refl::property(&cfarCfg::mode, "mode"),
      refl::property(&cfarCfg::noiseWindow, "noiseWindow"),
      refl::property(&cfarCfg::guardLength, "guardLength"),
      refl::property(&cfarCfg::shiftDivisor, "shiftDivisor"),
      refl::property(&cfarCfg::cyclic, "cyclic"),
      refl::property(&cfarCfg::threshold, "threshold"),
      refl::property(&cfarCfg::peakGrouping, "peakGrouping"));
};

struct cfarResponse {
  bool accepted;
  int appliedThreshold;

  constexpr static auto properties =
      std::make_tuple(refl::property(&cfarResponse::accepted, "accepted"),
                      refl::property(&cfarResponse::appliedThreshold,
                                     "appliedThreshold"));
};

struct sensorCfg {
  int sensorId;
  bool enabled;

  constexpr static std::string_view cmd{"sensorCfg"};

  constexpr static auto properties =
      std::make_tuple(refl::property(&sensorCfg::sensorId, "sensorId"),
                      refl::property(&sensorCfg::enabled, "enabled"));
};

struct sensorResponse {
  bool accepted;

  constexpr static auto properties =
      std::make_tuple(refl::property(&sensorResponse::accepted, "accepted"));
};

struct labelCmd {
  std::string_view label;

  constexpr static std::string_view cmd{"labelCmd"};

  constexpr static auto properties =
      std::make_tuple(refl::property(&labelCmd::label, "label"));
};

struct labelResponse {
  std::string_view echoed;

  constexpr static auto properties =
      std::make_tuple(refl::property(&labelResponse::echoed, "echoed"));
};

using registry_t =
    spie::command::registry<spie::command::binding<cfarCfg, cfarResponse>,
                            spie::command::binding<sensorCfg, sensorResponse>,
                            spie::command::binding<labelCmd, labelResponse>>;

cfarResponse handleCfar(const cfarCfg &cmd) {
  return cfarResponse{true, cmd.threshold};
}

sensorResponse handleSensor(const sensorCfg &cmd) {
  return sensorResponse{cmd.enabled};
}

labelResponse handleLabel(const labelCmd &cmd) {
  return labelResponse{cmd.label};
}

TEST_CASE("command parse text positional", "[command]") {
  auto parsed = spie::command::parse<registry_t>(
      "cfarCfg 1 2 3 4 5 6 7 8 9", spie::command::input_format::text);

  REQUIRE(parsed.ok());
  REQUIRE(std::holds_alternative<cfarCfg>(parsed.command));

  const auto &command = std::get<cfarCfg>(parsed.command);
  REQUIRE(command.subFrameIdx == 1);
  REQUIRE(command.threshold == 8);
  REQUIRE(command.peakGrouping == 9);
}

TEST_CASE("command parse from istream", "[command]") {
  std::istringstream stream{"sensorCfg 17 true"};
  auto parsed = spie::command::parse<registry_t>(
      stream, spie::command::input_format::text);

  REQUIRE(parsed.ok());
  REQUIRE(std::holds_alternative<sensorCfg>(parsed.command));

  const auto &command = std::get<sensorCfg>(parsed.command);
  REQUIRE(command.sensorId == 17);
  REQUIRE(command.enabled);
}

TEST_CASE("command parse json payload", "[command]") {
  auto parsed = spie::command::parse<registry_t>(
      R"({"cmd":"cfarCfg","payload":{"subFrameIdx":1,"direction":2,"mode":3,"noiseWindow":4,"guardLength":5,"shiftDivisor":6,"cyclic":7,"threshold":8,"peakGrouping":9}})",
      spie::command::input_format::json);

  REQUIRE(parsed.ok());
  REQUIRE(std::holds_alternative<cfarCfg>(parsed.command));

  const auto &command = std::get<cfarCfg>(parsed.command);
  REQUIRE(command.direction == 2);
  REQUIRE(command.threshold == 8);
}

TEST_CASE("command dispatch with delegates", "[command]") {
  spie::command::dispatcher<registry_t> dispatch;

  dispatch.connect<cfarCfg, handleCfar>();
  dispatch.connect<sensorCfg, handleSensor>();
  dispatch.connect<labelCmd, handleLabel>();

  auto parsed = spie::command::parse<registry_t>(
      "cfarCfg 1 2 3 4 5 6 7 88 9", spie::command::input_format::text);
  REQUIRE(parsed.ok());

  auto result = dispatch.dispatch(parsed.command);
  REQUIRE(result.ok());
  REQUIRE(std::holds_alternative<cfarResponse>(result.response));

  const auto &response = std::get<cfarResponse>(result.response);
  REQUIRE(response.accepted);
  REQUIRE(response.appliedThreshold == 88);
}

TEST_CASE("command parse quoted string into string_view with owned storage",
          "[command]") {
  auto parsed = spie::command::parse<registry_t>(
      "labelCmd \"front radar\"", spie::command::input_format::text);

  REQUIRE(parsed.ok());
  REQUIRE(std::holds_alternative<labelCmd>(parsed.command));

  const auto &command = std::get<labelCmd>(parsed.command);
  REQUIRE(command.label == "front radar");
}

TEST_CASE("command dispatch preserves parse storage for string_view responses",
          "[command]") {
  spie::command::dispatcher<registry_t> dispatch;
  dispatch.connect<labelCmd, handleLabel>();

  auto result = [&]() {
    auto parsed = spie::command::parse<registry_t>(
        "labelCmd \"front radar\"", spie::command::input_format::text);
    REQUIRE(parsed.ok());
    return dispatch.dispatch(parsed);
  }();

  REQUIRE(result.ok());
  REQUIRE(std::holds_alternative<labelResponse>(result.response));

  const auto &response = std::get<labelResponse>(result.response);
  REQUIRE(response.echoed == "front radar");

  auto rendered = spie::command::toText(result);
  REQUIRE(rendered.find("front radar") != std::string::npos);

  auto rendered_json = spie::command::toJsonObject(result).toString();
  REQUIRE(rendered_json.find("front radar") != std::string::npos);
}

TEST_CASE("command parse json supports string_view fields", "[command]") {
  auto parsed = spie::command::parse<registry_t>(
      R"({"cmd":"labelCmd","payload":{"label":"side radar"}})",
      spie::command::input_format::json);

  REQUIRE(parsed.ok());
  REQUIRE(std::holds_alternative<labelCmd>(parsed.command));
  REQUIRE(std::get<labelCmd>(parsed.command).label == "side radar");
}

TEST_CASE("command parse fails on arity mismatch", "[command]") {
  auto parsed = spie::command::parse<registry_t>(
      "cfarCfg 1 2 3", spie::command::input_format::text);

  REQUIRE_FALSE(parsed.ok());
  REQUIRE(parsed.error.code ==
          spie::command::parse_error_code::argument_count_mismatch);
}

TEST_CASE("command schema exposes method metadata", "[command]") {
  bool found_cfar = false;
  bool found_label = false;

  spie::command::schema::for_each_method<registry_t>([&](auto descriptor_id) {
    using descriptor = typename decltype(descriptor_id)::type;
    if constexpr (descriptor::name == std::string_view{"cfarCfg"}) {
      found_cfar = true;
      REQUIRE(descriptor::input_names.size() == 9U);
      REQUIRE(descriptor::output_names.size() == 2U);
    }

    if constexpr (descriptor::name == std::string_view{"labelCmd"}) {
      found_label = true;
      REQUIRE(descriptor::input_names[0] == "label");
      REQUIRE(descriptor::output_names[0] == "echoed");
    }
  });

  REQUIRE(found_cfar);
  REQUIRE(found_label);
}
