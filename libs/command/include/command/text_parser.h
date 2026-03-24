#pragma once

#include <string_view>
#include <vector>

#include <fsm/state_machine.h>

#include "tokenizer.h"
#include "types.h"

namespace spie::command::text {

using namespace spie::fsm;

class Context : public commandTokenizer {
public:
  Context(view &input) : commandTokenizer(input) {}
  Context(view &&input) : commandTokenizer(input) {}

  [[nodiscard]] bool atEnd() const {
    return getView().pos_ >= getView().input_.size();
  }

  void setCommandName(std::string_view name) { command_name_ = name; }

  void addArgument(std::string_view token) {
    if (token.size() >= 2U && token.front() == '"' && token.back() == '"') {
      arguments_.push_back(token.substr(1, token.size() - 2));
      return;
    }
    arguments_.push_back(token);
  }

  void setError(parse_error_code code, std::string message,
                std::size_t argument_index = 0U) {
    error_.code = code;
    error_.message = std::move(message);
    error_.position = getView().pos_;
    error_.argument_index = argument_index;
  }

  [[nodiscard]] std::string_view commandName() const { return command_name_; }
  [[nodiscard]] const std::vector<std::string_view> &arguments() const {
    return arguments_;
  }
  [[nodiscard]] const parse_error &error() const { return error_; }

private:
  std::string_view command_name_{};
  std::vector<std::string_view> arguments_{};
  parse_error error_{};
};

struct Initial;
struct CommandName;
struct Arguments;
struct Finished;
struct Error;

using States = spie::fsm::states<Initial, CommandName, Arguments, Finished, Error>;
using Machine = spie::fsm::StateMachine<States, Context>;

struct Initial : spie::fsm::state<Initial, Machine> {
  using state<Initial, Machine>::state;
  auto transitionInternalTo() -> transitions<CommandName, Error> const;
};

struct CommandName : spie::fsm::state<CommandName, Machine> {
  using state<CommandName, Machine>::state;
  auto transitionInternalTo() -> transitions<Arguments, Error> const;
};

struct Arguments : spie::fsm::state<Arguments, Machine> {
  using state<Arguments, Machine>::state;
  auto transitionInternalTo() -> transitions<Arguments, Finished, Error> const;
};

struct Finished : spie::fsm::state<Finished, Machine> {
  using state<Finished, Machine>::state;
  void onEnter() {}
};

struct Error : spie::fsm::state<Error, Machine> {
  using state<Error, Machine>::state;
  void onEnter() {}
};

} // namespace spie::command::text
