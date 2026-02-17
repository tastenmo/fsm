#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <json/value.h>
#include <sstream>
#include <string>

using namespace spie::json;

namespace {

std::string load_file(const std::filesystem::path &path) {
  std::ifstream ifs(path, std::ios::binary);
  if (!ifs) {
    throw std::runtime_error(std::string("Failed to open: ") + path.string());
  }

  std::ostringstream buffer;
  buffer << ifs.rdbuf();
  return buffer.str();
}

struct ParseResult {
  bool finished;
  bool errored;
  std::size_t pos;
  std::size_t variant_index;
};

ParseResult parse_and_diagnose(std::string_view input,
                               const std::string &name) {
  view v(input);
  value::Context ctx(v);
  auto fsm = value::Machine(mpl::type_identity<value::States>{}, ctx);
  fsm.emplace<value::Initial>();

  std::cout << "[" << name << "] ";
  std::cout << "Size: " << input.size() << " bytes, ";

  if (fsm.is_in<value::Finished>()) {
    std::cout << "✓ FINISHED" << std::endl;
    return {true, false, v.pos_, 0};
  } else if (fsm.is_in<value::Error>()) {
    std::cout << "✗ ERROR at pos " << v.pos_ << "/" << input.size()
              << std::endl;

    // Show context around error position
    const auto pos = v.pos_;
    const auto start = pos > 40 ? pos - 40 : 0;
    const auto end = std::min(pos + 40, input.size());
    std::cout << "  Context: ...";
    for (size_t i = start; i < end; ++i) {
      const char c = input[i];
      if (c == '\n') {
        std::cout << "\\n";
      } else if (c == '\r') {
        std::cout << "\\r";
      } else if (c == '\t') {
        std::cout << "\\t";
      } else if (c >= 32 && c < 127) {
        std::cout << c;
      } else {
        std::cout << "?";
      }
    }
    std::cout << "..." << std::endl;
    std::cout << "  Pos marker: ";
    for (size_t i = start; i < pos; ++i) {
      std::cout << " ";
    }
    std::cout << "^" << std::endl;

    return {false, true, v.pos_, 0};
  } else {
    std::cout << "? UNKNOWN STATE" << std::endl;
    return {false, false, v.pos_, 0};
  }
}

} // namespace

TEST_CASE("Real data - benchmark files", "[json][realdata]") {
  const std::filesystem::path data_dir =
      std::filesystem::path(__FILE__).parent_path() / "data" / "nativejson";

  // Test canada.json
  {
    auto canada = load_file(data_dir / "canada.json");
    auto result = parse_and_diagnose(canada, "canada.json");
    REQUIRE(result.finished);
    REQUIRE(!result.errored);
  }

  // Test citm_catalog.json
  {
    auto citm = load_file(data_dir / "citm_catalog.json");
    auto result = parse_and_diagnose(citm, "citm_catalog.json");
    REQUIRE(result.finished);
    REQUIRE(!result.errored);
  }

  // Test twitter.json
  {
    auto twitter = load_file(data_dir / "twitter.json");
    auto result = parse_and_diagnose(twitter, "twitter.json");
    REQUIRE(result.finished);
    REQUIRE(!result.errored);
  }
}

TEST_CASE("Edge cases - special structures", "[json][edgecase]") {
  std::pair<const char *, const char *> cases[] = {
      {"empty object", R"({})"},
      {"empty array", R"([])"},
      {"object with space", R"({ })"},
      {"array with space", R"([ ])"},
      {"nested empty", R"({"a":{}})"},
      {"nested empty array", R"({"a":[]})"},
      {"mixed nesting", R"({"a":[{},[]]})"},
      {"array of empty objects", R"([{},{},{}])"},
      {"object with newlines", "{\n\n}"},
      {"object with many newlines", "{\n\n\n\n}"},
      {"array with newlines", "[\n\n]"},
      {"deep nesting", R"({"a":{"b":{"c":{"d":{"e":"value"}}}}})"},
      {"deep array nesting", R"([[[[[1]]]]])"},
  };

  for (const auto &[name, json] : cases) {
    SECTION(name) {
      view v(json);
      value::Context ctx(v);
      auto fsm = value::Machine(mpl::type_identity<value::States>{}, ctx);
      fsm.emplace<value::Initial>();

      REQUIRE(fsm.is_in<value::Finished>());
      REQUIRE(!fsm.is_in<value::Error>());
    }
  }
}
