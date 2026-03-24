#include <command/text_parser.h>

namespace spie::command::text {

auto Initial::transitionInternalTo() -> transitions<CommandName, Error> const {
  context().consume(commandTokenType::WS);

  if (context().atEnd()) {
    context().setError(parse_error_code::empty_input, "command input is empty");
    return transition<Error>();
  }

  return transition<CommandName>();
}

auto CommandName::transitionInternalTo() -> transitions<Arguments, Error> const {
  if (auto token = context().consume(commandTokenType::IDENTIFIER)) {
    context().setCommandName(*token);
    return transition<Arguments>();
  }

  context().setError(parse_error_code::invalid_syntax,
                     "expected command identifier");
  return transition<Error>();
}

auto Arguments::transitionInternalTo() -> transitions<Arguments, Finished, Error> const {
  context().consume(commandTokenType::WS);

  if (context().atEnd()) {
    return transition<Finished>();
  }

  if (auto token = context().consume(commandTokenType::BOOLEAN_LITERAL)) {
    context().addArgument(*token);
    return transition<Arguments>();
  }

  if (auto token = context().consume(commandTokenType::FLOAT_LITERAL)) {
    context().addArgument(*token);
    return transition<Arguments>();
  }

  if (auto token = context().consume(commandTokenType::INT_LITERAL)) {
    context().addArgument(*token);
    return transition<Arguments>();
  }

  if (auto token = context().consume(commandTokenType::QUOTED_STRING)) {
    context().addArgument(*token);
    return transition<Arguments>();
  }

  if (auto token = context().consume(commandTokenType::IDENTIFIER)) {
    context().addArgument(*token);
    return transition<Arguments>();
  }

  if (auto token = context().consume(commandTokenType::BARE_STRING)) {
    context().addArgument(*token);
    return transition<Arguments>();
  }

  context().setError(parse_error_code::invalid_syntax, "invalid command argument token");
  return transition<Error>();
}

} // namespace spie::command::text
