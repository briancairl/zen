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

// a sequence of elements "stored" in a variadic pack with some hash helpers
template <typename T, T... Elements> struct sequence
{
public:
  static constexpr std::size_t hash() { return hash_storage; }
  constexpr operator std::size_t() const { return hash(); }

private:
  static constexpr std::size_t hash_storage = hash_sequence<T, Elements...>();
};

template <typename T, T... Elements> [[nodiscard]] constexpr std::size_t hash(const sequence<T, Elements...>& seq)
{
  return seq.hash();
}

// deduced difference elements for lhs/rhs sequences
template <typename T, T... LHS_Ts, T... RHS_Ts>
constexpr bool operator==(const sequence<T, LHS_Ts...>& lhs, const sequence<T, RHS_Ts...>& rhs)
{
  return false;
}

// deduced same elements for lhs/rhs sequences
template <typename T, T... Both_Ts>
constexpr bool operator==(const sequence<T, Both_Ts...>& lhs, const sequence<T, Both_Ts...>& rhs)
{
  return true;
}

// not equal, lifts above ==
template <typename T, T... LHS_Ts, T... RHS_Ts>
constexpr bool operator!=(const sequence<T, LHS_Ts...>& lhs, const sequence<T, RHS_Ts...>& rhs)
{
  return !operator==(lhs, rhs);
}

// a string-like sequence formed from a variadic pack, base on sequence
template <char... Elements> struct message : public sequence<char, Elements...>
{
public:
  static constexpr const char* c_str() { return str_storage; }
  constexpr operator const char*() const { return message::c_str(); }
  constexpr operator std::string_view() const { return std::string_view{message::c_str()}; }

private:
  static constexpr char str_storage[] = {Elements..., '\0'};
};

// custom literal for flare
template <typename T, T... Elements> constexpr auto operator""_msg() { return message<Elements...>{}; }

class status
{
public:
  static constexpr auto valid = "valid"_msg;
  static constexpr auto invalid = "invalid"_msg;
  static constexpr auto unknown = "unknown"_msg;

  constexpr status() = default;
  constexpr status(const status& other) = default;
  constexpr status(status&& other) : message_{other.message_} { other.message_ = unknown; }

  template <char... C> constexpr status(message<C...> m) : message_{m.c_str()} {}

  constexpr status& operator=(const status&) = default;
  constexpr status& operator=(status&&) = default;

  [[nodiscard]] constexpr std::string_view message() const { return message_; }
  [[nodiscard]] constexpr bool is_valid() const { return message_ == static_cast<std::string_view>(valid); }
  [[nodiscard]] constexpr operator bool() const { return status::is_valid(); }

private:
  std::string_view message_ = unknown;
};

inline std::size_t hash(const std::string_view sv)
{
  // https://stackoverflow.com/questions/7666509/hash-function-for-string
  constexpr std::size_t Shift = 5;  // ?

  std::size_t h = 0;
  for (auto itr = sv.rbegin(); itr != sv.rend(); ++itr)
  {
    h = ((h << Shift) + h) + static_cast<std::size_t>(*itr);
  }
  return h;
}

inline std::size_t hash(const status& s) { return hash(s.message()); }

template <char... C> inline bool operator==(const message<C...>& lhs, const status& rhs)
{
  return hash(lhs) == hash(rhs);
}

template <char... C> inline bool operator==(const status& lhs, const message<C...>& rhs)
{
  return hash(lhs) == hash(rhs);
}

template <char... C> inline bool operator!=(const message<C...>& lhs, const status& rhs) { return !(lhs == rhs); }

template <char... C> inline bool operator!=(const status& lhs, const message<C...>& rhs) { return !(lhs == rhs); }

inline std::ostream& operator<<(std::ostream& os, const status& s) { return os << s.message(); }

template <char... C> inline std::ostream& operator<<(std::ostream& os, const message<C...> m)
{
  return os << static_cast<std::string_view>(m);
}

}  // namespace zen
