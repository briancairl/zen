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
 * @brief Implements invocable dispatch behavior for free-function <code>all</code>
 *
 * Executes invocables until any of them returns an invalid result, then terminates, returning a result
 * holding the first invalid status. Otherwise, returns a result containing values returned by all invocables.
 * \n
 * This is the default, single threaded implementation. Specializations of all_dispatch may be made
 * available for invocation in different execution contexts, such as multi-threaded dispatch.
 */
template <typename... InvocableTs> class all_dispatch
{
public:
  explicit constexpr all_dispatch(InvocableTs&&... fs) : invocables_{std::forward<InvocableTs>(fs)...}
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
    return create(
      make_deferred_result(std::forward<InvocableTs>(std::get<Is>(invocables_)), std::forward_as_tuple(values...))...);
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
