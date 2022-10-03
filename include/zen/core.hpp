#pragma once

// C++ Standard Library
#include <utility>

// Zen
#include <zen/core/all_dispatch.hpp>
#include <zen/core/any_dispatch.hpp>

namespace zen
{

/**
 * @brief Passes values as a <code>result</code>
 */
template <typename... ValueTs> constexpr decltype(auto) pass(ValueTs&&... values)
{
  return make_result(std::forward<ValueTs>(values)...);
}

/**
 * @brief Executes invocables until any of them returns an invalid result<T>, then terminates, returning a result<T>
 * holding the first invalid status.
 *
 * Otherwise, returns a result<T> containing values returned by all invocables.
@verbatim
  auto r = pass(1, 2, 3)
         | any(
            [](int a, int b, int c) -> result<int> { return std::max({a, b, c}); },
            [](int a, int b, int c) -> result<int> { return std::min({a, b, c}); },
            [](int a, int b, int c) -> result<int> { return "failure"_msg; }
          );

  std::cout << std::boolalpha << static_cast<bool>(r) << std::endl;  // true
@endverbatim
 */
template <typename... InvocableTs> constexpr decltype(auto) any(InvocableTs&&... t)
{
  return any_dispatch<std::remove_reference_t<InvocableTs>...>{std::forward<InvocableTs>(t)...};
}

/**
 * @brief Executes invocables until any of them returns a valid result<T>, then terminates, returning that result<T>.
 *
 * Otherwise, returns result<T> with the last invalid status.
@verbatim
  auto r = pass(1, 2, 3)
         | all(
            [](int a, int b, int c) -> result<int> { return std::max({a, b, c}); },
            [](int a, int b, int c) -> result<int> { return std::min({a, b, c}); },
            [](int a, int b, int c) -> result<int> { return "failure"_msg; }
          );

  std::cout << std::boolalpha << static_cast<bool>(r) << std::endl;  // false
@endverbatim
 */
template <typename... InvocableTs> constexpr decltype(auto) all(InvocableTs&&... t)
{
  return all_dispatch<std::remove_reference_t<InvocableTs>...>{std::forward<InvocableTs>(t)...};
}

/**
 * @brief Chains together invocables which return a <code>result<T></code>
 *
 * Short-circuits when an invalid <code>result<T></code> is produce, and returns a <code>result<T></code>
 * with the invalid status causing failure.
 *
@verbatim
  auto r = pass(1, 2, 3)
         | [](int a, int b, int c) -> result<int> { return std::max({a, b, c}); }, // <-- executed
         | [](int a, int b, int c) -> result<int> { return "failure"_msg; }        // <-- execution stops here
           [](int a, int b, int c) -> result<int> { return std::min({a, b, c}); }; // <-- never invoked

  std::cout << std::boolalpha << static_cast<bool>(r) << std::endl;  // false
@endverbatim
 */
template <typename T, typename Fn> decltype(auto) operator|(result<T>&& r, Fn&& f)
{
  using original_return_type = decltype(std::apply(std::forward<Fn>(f), as_tuple(r)));
  using return_type = to_result_t<original_return_type>;
  return r.valid() ? return_type{std::apply(std::forward<Fn>(f), as_tuple(r))} : return_type{r.status()};
}

}  // namespace zen
