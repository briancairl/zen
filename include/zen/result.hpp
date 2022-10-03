#pragma once

// C++ Standard Library
#include <exception>
#include <iosfwd>
#include <utility>

// Zen
#include <zen/fwd.hpp>
#include <zen/meta/is_specialization.hpp>
#include <zen/meta/type_to_string.hpp>
#include <zen/result/deferred_result.hpp>
#include <zen/result/status.hpp>
#include <zen/result/to_result.hpp>
#include <zen/utility/value_mem.hpp>

namespace zen
{

/**
 * @brief Exception thrown when result does not contain a valid value
 */
class bad_result_access final : std::exception
{
public:
  explicit bad_result_access(const char* const type_info) : type_info_{type_info} {}

private:
  const char* what() const noexcept override { return type_info_; }
  const char* type_info_;
};

/**
 * @brief Stores a value type, or an error message
 *
 * @tparam T  value type
 */
template <typename T> class result final : private value_mem<T>
{
public:
  /**
   * @brief Creates an invalid result from an error message
   *
   * @param error_message  error description message; must not be <code>zen::Valid</code>
   */
  template <char... Elements> constexpr result(message<Elements...>&& error_message);

  /**
   * @brief Creates an invalid result from an error status
   *
   * @param status  error status
   */
  constexpr result(result_status&& status);

  /**
   * @brief Creates a valid result from a value
   *
   * @param value  value payload
   */
  constexpr result(const T& value);

  /**
   * @copydoc result(const T&)
   */
  constexpr result(T&& value);

  /**
   * @brief Default initializes result to invalid (Unknown) state
   */
  constexpr result() = default;

  /**
   * @brief Destroys result value, if <code>valid() == true</code>
   */
  ~result();

  /**
   * @brief Returns immutable reference to result value
   *
   * @note use operator* as a more efficient option that does not check validity
   *
   * @throws bad_result_access  if result is not valid
   */
  [[nodiscard]] const T& value() const&;

  /**
   * @brief Returns rvalue reference to result value
   *
   * @note use operator* as a more efficient option that does not check validity
   *
   * @throws bad_result_access  if result is not valid
   */
  [[nodiscard]] T&& value() &&;

  /**
   * @brief Returns status associated with result
   */
  [[nodiscard]] constexpr result_status status() const;

  /**
   * @brief Returns <code>true</code> if result value pay load is valid
   *
   * If <code>valid() == true</code>, then <code>status().message() == Valid</code>
   */
  [[nodiscard]] constexpr bool valid() const;

  /**
   * @copydoc valid
   *
   * Enables implicit cast to <code>bool</code> based on valid()
   */
  [[nodiscard]] constexpr operator bool() const;

  using value_mem<T>::operator*;
  using value_mem<T>::operator->;

private:
  result_status status_;
};

template <typename T>
template <char... Elements>
constexpr result<T>::result(message<Elements...>&& error_message) : value_mem<T>{}, status_{error_message}
{
  static_assert(
    !are_messages_equal<message<Elements...>, decltype(Valid)>(),
    "To set a valid result, assign a value, not an error message");
}

template <typename T> constexpr result<T>::result(result_status&& status) : value_mem<T>{}, status_{std::move(status)}
{}

template <typename T> constexpr result<T>::result(const T& value) : value_mem<T>{value}, status_{Valid} {}

template <typename T> constexpr result<T>::result(T&& value) : value_mem<T>{std::move(value)}, status_{Valid} {}

template <typename T> result<T>::~result()
{
  if (status_.valid())
  {
    result::destroy();
  }
}

template <typename T> const T& result<T>::value() const&
{
  if (!status_.valid())
  {
    throw bad_result_access{meta::type_to_string<T>()};
  }
  return **this;
}

template <typename T> T&& result<T>::value() &&
{
  if (!status_.valid())
  {
    throw bad_result_access{meta::type_to_string<T>()};
  }
  status_ = Unknown;
  return std::move(**this);
}

template <typename T> constexpr result_status result<T>::status() const { return status_; }

template <typename T> constexpr bool result<T>::valid() const { return status_.valid(); }

template <typename T> constexpr result<T>::operator bool() const { return status_.valid(); }

template <typename T> [[nodiscard]] constexpr decltype(auto) as_tuple(const result<T>& r)
{
  return std::forward_as_tuple(*r);
}

template <typename... Ts> [[nodiscard]] constexpr decltype(auto) as_tuple(const result<std::tuple<Ts...>>& r)
{
  return *r;
}

template <typename T> [[nodiscard]] constexpr decltype(auto) as_tuple(result<T>&& r)
{
  return std::forward_as_tuple(*r);
}

template <typename... Ts> [[nodiscard]] constexpr decltype(auto) as_tuple(result<std::tuple<Ts...>>&& r) { return *r; }

/**
 * @brief Traits type which isolates the type returned by <code>result::value()</code> as a <code>std::tuple</code>
 */
template <typename ResultT> struct result_as_tuple;

/**
 * @copydoc result_as_tuple
 */
template <typename T> struct result_as_tuple<result<T>>
{
  using type = std::tuple<T>;
};

/**
 * @copydoc result_as_tuple
 */
template <typename... Ts> struct result_as_tuple<result<std::tuple<Ts...>>>
{
  using type = std::tuple<Ts...>;
};

/**
 * @copydoc result_as_tuple
 */
template <typename ResultT> using result_as_tuple_t = typename result_as_tuple<ResultT>::type;

/**
 * @brief Wraps values or result_status in a result
 *
 * @param values...  values to wrap, or result_status
 *
 * @return result<Ts...>
 */
template <typename... Ts> decltype(auto) make_result(Ts&&... values)
{
  static_assert(sizeof...(Ts) > 0, "at least one value must be specified");

  if constexpr (sizeof...(Ts) > 1)
  {
    return result<std::tuple<std::remove_reference_t<Ts>...>>{{std::forward<Ts>(values)...}};
  }
  else if constexpr (meta::is_specialization_v<std::remove_reference_t<Ts>..., std::tuple>)
  {
    return result<std::remove_reference_t<Ts>...>{std::forward<Ts>(values)...};
  }
  else
  {
    return result<std::remove_reference_t<Ts>...>{std::forward<Ts>(values)...};
  }
}

/**
 * @brief <code>std::ostream</code> overload for <code>result<T></code>
 *
 * Puts <code>r</code> into <code>os</code> if valid, otherwise puts <code>r.status</code>
 *
 * @tparam T  value type
 *
 * @param[in,out] os  output stream
 * @param r  result
 *
 * @return os
 */
template <typename T> inline std::ostream& operator<<(std::ostream& os, const result<T>& r)
{
  if (r.valid())
  {
    return os << *r;
  }
  return os << r.status();
}

}  // namespace zen
