#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <json/value.h>

using namespace spie::json;

namespace {

struct CoutStateGuard {
  explicit CoutStateGuard() : state_(std::cout.rdstate()) {
    std::cout.setstate(std::ios_base::failbit);
  }

  ~CoutStateGuard() { std::cout.clear(state_); }

  std::ios_base::iostate state_;
};

std::string load_file(const std::filesystem::path &path) {
  std::ifstream ifs(path, std::ios::binary);
  REQUIRE(ifs);

  std::ostringstream buffer;
  buffer << ifs.rdbuf();
  return buffer.str();
}

struct ParseResult {
  bool finished;
  std::size_t variant_index;
};

ParseResult parse_json(std::string_view input) {
  view v(input);
  value::Context ctx(v);
  auto fsm = value::Machine(mpl::type_identity<value::States>{}, ctx);
  fsm.emplace<value::Initial>();
  auto result = ctx.getValue();
  return {fsm.is_in<value::Finished>(), result.getVariant().index()};
}

} // namespace

TEST_CASE("JSON benchmark - nativejson", "[json][benchmark]") {
  CoutStateGuard cout_guard;

  const std::filesystem::path data_dir =
      std::filesystem::path(__FILE__).parent_path() / "data" / "nativejson";

  const auto canada = load_file(data_dir / "canada.json");
  const auto citm_catalog = load_file(data_dir / "citm_catalog.json");
  const auto twitter = load_file(data_dir / "twitter.json");

  const auto canada_result = parse_json(canada);
  const auto citm_result = parse_json(citm_catalog);
  const auto twitter_result = parse_json(twitter);

  REQUIRE(canada_result.finished);
  REQUIRE(citm_result.finished);
  REQUIRE(twitter_result.finished);

  BENCHMARK("parse canada.json") {
    auto result = parse_json(canada);
    return result.variant_index + static_cast<std::size_t>(result.finished);
  };

  BENCHMARK("parse citm_catalog.json") {
    auto result = parse_json(citm_catalog);
    return result.variant_index + static_cast<std::size_t>(result.finished);
  };

  BENCHMARK("parse twitter.json") {
    auto result = parse_json(twitter);
    return result.variant_index + static_cast<std::size_t>(result.finished);
  };
}
