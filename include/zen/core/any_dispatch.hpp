#pragma once

// C++ Standard Library
#include <tuple>
#include <utility>

// Zen
#include <zen/fwd.hpp>
#include <zen/meta/first.hpp>
#include <zen/result.hpp>

namespace zen
{

/**
 * @brief Implements invocable dispatch behavior for free-function <code>any</code>
 *
 * Executes invocables until any of them returns a valid result, then terminates, returning that result.
 * Otherwise, returns result with the last invalid status.
 * \n
 * This is the default, single threaded implementation. Specializations of any_dispatch may be made
 * available for invocation in different execution contexts, such as multi-threaded dispatch.
 */
template <typename... InvocableTs> class any_dispatch
{
public:
  explicit constexpr any_dispatch(InvocableTs&&... fs) : invocables_{std::forward<InvocableTs>(fs)...}
  {
    static_assert(sizeof...(InvocableTs) > 0, "At least one invocable must be specified");
  };

  /**
   * @brief Invokes all held invocables with <code>values</code> as arguments
   *
   * @return result
   */
  template <typename... ValueTs> decltype(auto) operator()(ValueTs&&... values) const
  {
    static constexpr std::size_t N = sizeof...(InvocableTs);
    return call_exec_impl(std::make_index_sequence<N>{}, std::forward<ValueTs>(values)...);
  }

private:
  template <typename... ValueTs, std::size_t... Is>
  decltype(auto) exec_impl(std::index_sequence<Is...> _, ValueTs&&... values) const
  {
    using result_type = std::invoke_result_t<meta::first_t<InvocableTs...>, ValueTs...>;

    static_assert(
      (sizeof...(InvocableTs) == 1) ||
        (std::is_same_v<result_type, std::invoke_result_t<InvocableTs, ValueTs...>> && ...),
      "'InvocableTs' executed under [any_dispatch] must all have the same return type");

    result_type return_value;
    {
      [[maybe_unused]] const auto unused =
        ((return_value = std::get<Is>(this->invocables_)(std::forward<ValueTs>(values)...), return_value.valid()) ||
         ...);
    }
    return return_value;
  }

  template <typename... ValueTs, typename Ignore, std::size_t... Is>
  decltype(auto) exec_impl_ignore(std::index_sequence<Is...> _, Ignore&&, ValueTs&&... values) const
  {
    return exec_impl(_, std::forward<ValueTs>(values)...);
  }

  template <typename... ValueTs, std::size_t... Is>
  decltype(auto) call_exec_impl(std::index_sequence<Is...> _, ValueTs&&... values) const
  {
    if constexpr (std::is_base_of_v<
                    exec::executor_handle<std::remove_reference_t<meta::first_t<ValueTs...>>>,
                    std::remove_reference_t<meta::first_t<ValueTs...>>>)
    {
      return exec_impl_ignore(_, std::forward<ValueTs>(values)...);
    }
    else
    {
      return exec_impl(_, std::forward<ValueTs>(values)...);
    }
  }

  std::tuple<InvocableTs&&...> invocables_;
};

}  // namespace zen
