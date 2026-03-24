#include <catch2/catch_all.hpp>

#include <ctre.hpp>

#include <map>
#include <reflect/property.h>
#include <reflect_json/reflect_json.h>
#include <vector>

using namespace std::string_view_literals;

struct JsonStruct {
  int id;
  bool enabled;
  std::string name;

  constexpr static auto properties =
      std::make_tuple(spie::reflect::property(&JsonStruct::id, "id"sv),
                      spie::reflect::property(&JsonStruct::enabled, "enabled"sv),
                      spie::reflect::property(&JsonStruct::name, "name"sv));
};

TEST_CASE("Reflect JSON Roundtrip", "[reflect_json]") {
  JsonStruct in{42, true, "demo"};

  const auto jsonObject = spie::reflect::json_adapter::toJsonObject(in);
  const auto out =
      spie::reflect::json_adapter::fromJsonObject<JsonStruct>(jsonObject);

  REQUIRE(out.has_value());
  REQUIRE(out->id == 42);
  REQUIRE(out->enabled == true);
  REQUIRE(out->name == "demo");
}

TEST_CASE("Reflect JSON Missing Field", "[reflect_json]") {
  spie::json::jsonObject obj;
  obj.addValue({"id", spie::json::jsonValue{spie::json::number::JsonNumber{7}}});
  obj.addValue({"enabled", spie::json::jsonValue{true}});

  const auto out =
      spie::reflect::json_adapter::fromJsonObject<JsonStruct>(obj);
  REQUIRE_FALSE(out.has_value());
}

struct NestedChild {
  int count;

  constexpr static auto properties =
      std::make_tuple(spie::reflect::property(&NestedChild::count,
                                              ctll::fixed_string{"count"}));
};

struct NestedRoot {
  std::string name;
  std::optional<int> retry;
  NestedChild child;

  constexpr static auto properties = std::make_tuple(
      spie::reflect::property(&NestedRoot::name, ctll::fixed_string{"name"}),
      spie::reflect::property(&NestedRoot::retry, "retry"sv),
      spie::reflect::property(&NestedRoot::child, ctll::fixed_string{"child"}));
};

TEST_CASE("Reflect JSON Nested Roundtrip", "[reflect_json]") {
  NestedRoot input{"worker", 3, NestedChild{9}};

  auto jsonObj = spie::reflect::json_adapter::toJsonObject(input);
  auto parsed =
      spie::reflect::json_adapter::fromJsonObject<NestedRoot>(jsonObj);

  REQUIRE(parsed.has_value());
  REQUIRE(parsed->name == "worker");
  REQUIRE(parsed->retry.has_value());
  REQUIRE(*parsed->retry == 3);
  REQUIRE(parsed->child.count == 9);
}

TEST_CASE("Reflect JSON Optional Missing Is Nullopt", "[reflect_json]") {
  spie::json::jsonObject root;
  root.addValue({"name", spie::json::jsonValue{std::string{"worker"}}});

  spie::json::jsonObject child;
  child.addValue(
      {"count", spie::json::jsonValue{spie::json::number::JsonNumber{11}}});
  root.addValue({"child", spie::json::jsonValue{child}});

  auto parsed = spie::reflect::json_adapter::fromJsonObject<NestedRoot>(root);
  REQUIRE(parsed.has_value());
  REQUIRE(parsed->name == "worker");
  REQUIRE_FALSE(parsed->retry.has_value());
  REQUIRE(parsed->child.count == 11);
}

TEST_CASE("Reflect JSON Permissive Policy Keeps Defaults", "[reflect_json]") {
  spie::json::jsonObject root;
  root.addValue({"name", spie::json::jsonValue{std::string{"worker"}}});

  auto parsed = spie::reflect::json_adapter::fromJsonObject<
      NestedRoot, spie::reflect::json_adapter::permissive_policy>(
      root, spie::reflect::json_adapter::permissive_policy{});

  REQUIRE(parsed.has_value());
  REQUIRE(parsed->name == "worker");
  REQUIRE_FALSE(parsed->retry.has_value());
  REQUIRE(parsed->child.count == 0);
}

struct CollectionRoot {
  std::vector<int> ids;
  std::vector<NestedChild> children;
  std::map<std::string, int> weights;
  std::map<std::string, NestedChild> namedChildren;

  constexpr static auto properties = std::make_tuple(
      spie::reflect::property(&CollectionRoot::ids, "ids"sv),
      spie::reflect::property(&CollectionRoot::children, "children"sv),
      spie::reflect::property(&CollectionRoot::weights, "weights"sv),
      spie::reflect::property(&CollectionRoot::namedChildren,
                              "namedChildren"sv));
};

TEST_CASE("Reflect JSON Collection Roundtrip", "[reflect_json]") {
  CollectionRoot input{{1, 2, 3},
                       {NestedChild{4}, NestedChild{5}},
                       {{"light", 7}, {"heavy", 11}},
                       {{"first", NestedChild{8}}, {"second", NestedChild{9}}}};

  auto jsonObj = spie::reflect::json_adapter::toJsonObject(input);
  auto parsed =
      spie::reflect::json_adapter::fromJsonObject<CollectionRoot>(jsonObj);

  REQUIRE(parsed.has_value());
  REQUIRE(parsed->ids == std::vector<int>{1, 2, 3});
  REQUIRE(parsed->children.size() == 2u);
  REQUIRE(parsed->children[0].count == 4);
  REQUIRE(parsed->children[1].count == 5);
  REQUIRE(parsed->weights.size() == 2u);
  REQUIRE(parsed->weights.at("light") == 7);
  REQUIRE(parsed->weights.at("heavy") == 11);
  REQUIRE(parsed->namedChildren.size() == 2u);
  REQUIRE(parsed->namedChildren.at("first").count == 8);
  REQUIRE(parsed->namedChildren.at("second").count == 9);
}

TEST_CASE("Reflect JSON Collection Type Mismatch Fails", "[reflect_json]") {
  spie::json::jsonObject root;

  spie::json::jsonArray ids;
  ids.addValue(spie::json::jsonValue{spie::json::number::JsonNumber{1}});
  ids.addValue(spie::json::jsonValue{std::string{"bad"}});
  root.addValue({"ids", spie::json::jsonValue{ids}});

  spie::json::jsonArray children;
  spie::json::jsonObject child;
  child.addValue(
      {"count", spie::json::jsonValue{spie::json::number::JsonNumber{3}}});
  children.addValue(spie::json::jsonValue{child});
  root.addValue({"children", spie::json::jsonValue{children}});

  spie::json::jsonObject weights;
  weights.addValue(
      {"light", spie::json::jsonValue{spie::json::number::JsonNumber{7}}});
  root.addValue({"weights", spie::json::jsonValue{weights}});

  spie::json::jsonObject namedChildren;
  namedChildren.addValue({"first", spie::json::jsonValue{child}});
  root.addValue({"namedChildren", spie::json::jsonValue{namedChildren}});

  auto parsed =
      spie::reflect::json_adapter::fromJsonObject<CollectionRoot>(root);
  REQUIRE_FALSE(parsed.has_value());
}
