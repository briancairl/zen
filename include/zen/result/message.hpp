#pragma once

// C++ Standard Library
#include <iosfwd>
#include <string_view>
#include <utility>

namespace zen
{

// base case
template <typename T, T FirstElement> constexpr std::size_t hash_sequence()
{
  return static_cast<std::size_t>(FirstElement);
}

// hashes a sequence of elements at compile time
template <typename T, T FirstElement, T SecondElement, T... OtherElements> constexpr std::size_t hash_sequence()
{
  // https://stackoverflow.com/questions/7666509/hash-function-for-string but at compile time
  constexpr std::size_t Shift = 5;  // ?
  constexpr std::size_t Hash = hash_sequence<T, SecondElement, OtherElements...>();
  return ((Hash << Shift) + Hash) + static_cast<std::size_t>(FirstElement);
}

// a string-like sequence formed from a variadic pack, base on sequence
template <char... Elements> struct message
{
public:
  static constexpr std::size_t hash() { return hash_storage; }
  static constexpr const char* c_str() { return str_storage; }
  static constexpr std::string_view sv() { return std::string_view{message::c_str()}; }

private:
  static constexpr std::size_t hash_storage = hash_sequence<char, Elements...>();
  static constexpr char str_storage[] = {Elements..., '\0'};
};

template <typename LSeq, typename RSeq> struct are_messages_equal : std::false_type
{};

template <char... Elements> struct are_messages_equal<message<Elements...>, message<Elements...>> : std::true_type
{};

template <typename T, T... Elements> constexpr auto operator""_msg() { return message<Elements...>{}; }

template <typename T, T... Elements> constexpr auto operator""_hash() { return message<Elements...>::hash(); }

template <char... C> inline std::ostream& operator<<(std::ostream& os, const message<C...> m) { return os << m.sv(); }

}  // namespace zen
