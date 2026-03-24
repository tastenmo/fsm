#pragma once

#include <optional>
#include <string>
#include <tuple>
#include <variant>

#include <core/utils.h>
#include <signal/delegate.h>

#include "registry.h"
#include "types.h"

namespace spie::command {

struct dispatch_error {
  std::string message{};
};

template <class Registry> struct dispatch_result {
  typename Registry::response_variant response{};
  std::optional<dispatch_error> error{};
  std::shared_ptr<parse_storage> storage{};

  [[nodiscard]] bool ok() const { return !error.has_value(); }
};

template <class Registry> class dispatcher;

template <class... Bindings> class dispatcher<registry<Bindings...>> {
public:
  using registry_type = registry<Bindings...>;
  using command_variant = typename registry_type::command_variant;
  using response_variant = typename registry_type::response_variant;

private:
  using handler_tuple =
      std::tuple<delegate<typename Bindings::response_type(
          const typename Bindings::command_type &)>...>;

public:
  template <class Command, auto Candidate> void connect() {
    get_handler<Command>().template connect<Candidate>();
  }

  template <class Command, auto Candidate, class Instance>
  void connect(Instance &instance) {
    get_handler<Command>().template connect<Candidate>(instance);
  }

  template <class Command>
  auto &get_handler() {
    return std::get<registry_type::template command_index<Command>>(handlers_);
  }

  template <class Command>
  const auto &get_handler() const {
    return std::get<registry_type::template command_index<Command>>(handlers_);
  }

  dispatch_result<registry_type> dispatch(const command_variant &command) const {
    dispatch_result<registry_type> result{};

    std::visit(
        overloaded{
            [&](const std::monostate &) {
              result.error = dispatch_error{"cannot dispatch empty command"};
            },
            [&](const auto &typed_command) {
              using command_type = std::decay_t<decltype(typed_command)>;
              constexpr auto idx =
                  registry_type::template command_index<command_type>;
              const auto &handler = std::get<idx>(handlers_);

              if (!handler) {
                result.error =
                    dispatch_error{"command handler is not connected"};
                return;
              }

              result.response = handler(typed_command);
            }},
        command);

    return result;
  }

  dispatch_result<registry_type>
  dispatch(const parse_result<registry_type> &parsed) const {
    if (!parsed.ok()) {
      return dispatch_result<registry_type>{
          {},
          dispatch_error{"cannot dispatch a parse result with errors"},
          parsed.storage};
    }

    auto result = dispatch(parsed.command);
    result.storage = parsed.storage;
    return result;
  }

private:
  handler_tuple handlers_{};
};

} // namespace spie::command
