
#include <core/type_traits.h>

#include <catch2/catch_test_macros.hpp>

#include <json/string.h>
#include <json/tokenizer.h>

using namespace spie::json;

using namespace std::literals;

TEST_CASE("Json_simple strings", "[json]") {

  auto fsm = string::Machine(
      mpl::type_identity<string::States>{},
      string::Context(
          view{"\"This is a simple string.\" this should be ignored."sv}));

  fsm.emplace<string::Initial>();

  REQUIRE(fsm.is_in<string::Finished>());

  REQUIRE(fsm.context().value() == "This is a simple string."sv);

  REQUIRE(fsm.context().size() == 24);
}

TEST_CASE("Json_strings with special characters", "[json]") {

  string::Context ctx(
      view{"\"This is a more complicated string:\\n"
           "\\tIt contains special character,\\n"
           "\\tcompiles in 10 \\u00B5s,\\n"
           "\\tand only needs \\u00BD the time.\" this should be ignored."sv});

  auto fsm = string::Machine(mpl::type_identity<string::States>{}, ctx);

  fsm.emplace<string::Initial>();

  REQUIRE(fsm.is_in<string::Finished>());

  //  REQUIRE(ctx.value() == "This is a simple string."sv);

  REQUIRE(ctx.size() == 130);
  auto value = ctx.value();
  CAPTURE(value);
}

TEST_CASE("Json_strings with Japanese characters and emojis", "[json]") {

  // Test string with Japanese characters, emojis, and escaped newlines
  string::Context ctx(view{
      "\"RT @AFmbsk: @samao21718 \\n呼び方☞まおちゃん\\n呼ばれ方☞あーちゃん\\n第一印象☞平野から？！\\n今の印象☞おとなっぽい！！\\nLINE交換☞もってるん\\\\( ˆoˆ )/\\nトプ画について☞楽しそうでいーな😳\\n家族にするなら☞おねぇちゃん\\n最後に一言☞全然会えない…\""sv});

  auto fsm = string::Machine(mpl::type_identity<string::States>{}, ctx);

  fsm.emplace<string::Initial>();

  REQUIRE(fsm.is_in<string::Finished>());

  auto value = ctx.value();

  // Verify the string contains expected Japanese characters and emoji
  REQUIRE(value.find("まおちゃん") != std::string_view::npos);
  REQUIRE(value.find("😳") != std::string_view::npos);
  REQUIRE(value.find("平野から？！") != std::string_view::npos);
}

TEST_CASE("Json_strings with emoji simple test", "[json]") {

  string::Context ctx(view{"\"Test with emoji 😳 here\""sv});

  auto fsm = string::Machine(mpl::type_identity<string::States>{}, ctx);

  fsm.emplace<string::Initial>();

  INFO("Is in Finished: " << fsm.is_in<string::Finished>());
  INFO("Parsed value: " << ctx.value());

  REQUIRE(fsm.is_in<string::Finished>());
  REQUIRE(ctx.value().find("😳") != std::string_view::npos);
}
