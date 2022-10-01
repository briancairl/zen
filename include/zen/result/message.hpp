#pragma once

// C++ Standard Library
#include <iosfwd>
#include <string_view>
#include <utility>

namespace zen
{
namespace detail
{

static constexpr std::size_t kStringHashShift = 7;

/**
 * @brief Computes a hash from elements in a variadic argument pack
 */
template <typename T, T FirstElement> constexpr std::size_t hash_sequence();

/**
 * @copydoc hash_sequence
 * @see https://stackoverflow.com/questions/7666509/hash-function-for-string
 */
template <typename T, T FirstElement, T SecondElement, T... OtherElements> constexpr std::size_t hash_sequence()
{

  constexpr std::size_t Hash = hash_sequence<T, SecondElement, OtherElements...>();
  return ((Hash << kStringHashShift) + Hash) + static_cast<std::size_t>(FirstElement);
}

/**
 * @copydoc hash_sequence
 * @note base case
 */
template <typename T, T FirstElement> constexpr std::size_t hash_sequence()
{
  return static_cast<std::size_t>(FirstElement);
}

}  // namespace detail

/**
 * @brief Holds characters in a variadic pack
 *
 * Acts as a string literal creator/wrapper with safeguards against invalid
 * c-string points
 *
 * @tparam Elements  characters of string
 */
template <char... Elements> struct message
{
public:
  /**
   * @brief Returns hash of string Elements
   */
  static constexpr std::size_t hash() { return hash_storage; }

  /**
   * @brief Returns C-string comprised of <code>{Elements...}</code>
   */
  static constexpr const char* c_str() { return str_storage; }

  /**
   * @brief Returns string literal storage as a <code>std::string_view</code>
   */
  static constexpr std::string_view sv() { return std::string_view{message::c_str()}; }

private:
  /// Compile-time hashing result storage
  static constexpr std::size_t hash_storage = detail::hash_sequence<char, Elements...>();

  /// String literal constant, formed from Elements
  static constexpr char str_storage[] = {Elements..., '\0'};
};

/**
 * @brief Creates a message from a string literal
 *
@verbatim
// Create the message constant
const auto m = "this is a message"_msg;

// Print associate string literal
std::fprintf(stderr, "%s\n", m.c_str());
@endverbatim
 */
template <typename T, T... Elements> constexpr auto operator""_msg() { return message<Elements...>{}; }

/**
 * @brief Returns hash value associate which a string literal
@verbatim
switch(hash(str))
{
case "one case"_hash:
  return 1;
case "two case"_hash:
  return 2;
...
}
@endverbatim
 */
template <typename T, T... Elements> constexpr auto operator""_hash() { return message<Elements...>::hash(); }

/**
 * @brief Checks if two <code>message</code> types are equal
 *
@verbatim
const auto m1 = "this is a message"_msg;
const auto m2 = "this is also a message"_msg;

// prints 'true'
std::cout << std::boolalpha << are_messages_equal<decltype(m1), decltype(m1)>() << std::endl;

// prints 'false'
std::cout << std::boolalpha << are_messages_equal<decltype(m1), decltype(m2)>() << std::endl;
@endverbatim
 */
template <typename LSeq, typename RSeq> struct are_messages_equal : std::false_type
{};

/**
 * @copydoc are_messages_equal
 * @note <code>true</code> case
 */
template <char... Elements> struct are_messages_equal<message<Elements...>, message<Elements...>> : std::true_type
{};

/**
 * @brief <code>std::ostream</code> overload for message
 */
template <char... C> inline std::ostream& operator<<(std::ostream& os, const message<C...> m) { return os << m.sv(); }

}  // namespace zen
