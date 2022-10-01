#pragma once

// C++ Standard Library
#include <iosfwd>
#include <string_view>
#include <utility>

// Zen
#include <zen/result/message.hpp>

namespace zen
{

/**
 * @brief Creates a failure message from a string literal
 */
template <typename T, T... Elements> constexpr auto operator""_fail() { return message<Elements...>{}; }

/**
 * @brief Standard message used to indicate valid status
 */
static constexpr auto Valid = "valid"_msg;

/**
 * @brief Standard message used to indicate invalid status
 */
static constexpr auto Invalid = "invalid"_msg;

/**
 * @brief Standard message used to indicate unknown status
 */
static constexpr auto Unknown = "unknown"_msg;

/**
 * @brief Indicates valid/invalid state with an associated message payload
 */
class result_status
{
public:
  constexpr result_status() = default;
  constexpr result_status(const result_status& other) = default;
  constexpr result_status(result_status&& other) : message_{other.message_} { other.message_ = Unknown.sv(); }
  constexpr result_status& operator=(const result_status&) = default;
  constexpr result_status& operator=(result_status&&) = default;

  /**
   * @brief Creates result_status from a message
   */
  template <char... C> constexpr result_status(message<C...> m) : message_{m.c_str()} {}

  /**
   * @brief Returns message string payload associated with status
   */
  [[nodiscard]] constexpr std::string_view message() const { return message_; }

  /**
   * @brief Returns <code>true</code> if status indicates "Valid" state
   */
  [[nodiscard]] constexpr bool valid() const { return message_.data() == Valid.c_str(); }

private:
  std::string_view message_ = Unknown.sv();
};

/**
 * @brief Computes a hash of a string
 *
 * @param sv  string to hash, as a <code>std::string_view</code>
 *
 * @return hash value
 *
 * @see https://stackoverflow.com/questions/7666509/hash-function-for-string
 */
inline std::size_t hash(const std::string_view sv)
{
  constexpr std::size_t Shift = 5;  // ?

  std::size_t h = 0;
  for (auto itr = sv.rbegin(); itr != sv.rend(); ++itr)
  {
    h = ((h << Shift) + h) + static_cast<std::size_t>(*itr);
  }
  return h;
}

/**
 * @brief Computes a hash associated with a result_status
 *
 * @param s  status object
 *
 * @return hash value
 *
 * @see https://stackoverflow.com/questions/7666509/hash-function-for-string
 */
inline std::size_t hash(const result_status& s) { return hash(s.message()); }

/**
 * @brief Equality comparions between message and result_status (with message)
 */
template <char... C> [[nodiscard]] constexpr bool operator==(const message<C...>& lhs, const result_status& rhs)
{
  return lhs.sv() == rhs.message();
}

/**
 * @brief Equality comparions between result_status (with message) and message
 */
template <char... C> [[nodiscard]] constexpr bool operator==(const result_status& lhs, const message<C...>& rhs)
{
  return lhs.message() == rhs.sv();
}

/**
 * @brief Inequality comparions between message and result_status (with message)
 */
template <char... C> [[nodiscard]] constexpr bool operator!=(const message<C...>& lhs, const result_status& rhs)
{
  return !(lhs == rhs);
}

/**
 * @brief Inequality comparions between result_status (with message) and message
 */
template <char... C> [[nodiscard]] constexpr bool operator!=(const result_status& lhs, const message<C...>& rhs)
{
  return !(lhs == rhs);
}

/**
 * @brief Equality comparions between two result_status objects
 */
[[nodiscard]] constexpr bool operator==(const result_status& lhs, const result_status& rhs)
{
  return lhs.message() == rhs.message();
}

/**
 * @brief Inequality comparions between two result_status objects
 */
[[nodiscard]] constexpr bool operator!=(const result_status& lhs, const result_status& rhs) { return !(lhs == rhs); }

/**
 * @brief <code>std::ostream</code> overload for result_status
 */
inline std::ostream& operator<<(std::ostream& os, const result_status& s) { return os << s.message(); }

}  // namespace zen
