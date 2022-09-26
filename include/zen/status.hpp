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

private:
  static constexpr std::size_t hash_storage = hash_sequence<T, Elements...>();
};

template <typename T, T... Elements> [[nodiscard]] constexpr std::size_t hash(const sequence<T, Elements...>& seq)
{
  return seq.hash();
}

// a string-like sequence formed from a variadic pack, base on sequence
template <char... Elements> struct message : public sequence<char, Elements...>
{
public:
  static constexpr const char* c_str() { return str_storage; }
  static constexpr std::string_view sv() { return std::string_view{message::c_str()}; }

private:
  static constexpr char str_storage[] = {Elements..., '\0'};
};

template <typename LSeq, typename RSeq> struct are_messages_equal : std::false_type
{};

template <char... Elements> struct are_messages_equal<message<Elements...>, message<Elements...>> : std::true_type
{};

template <typename T, T... Elements> constexpr auto operator""_msg() { return message<Elements...>{}; }

template <typename T, T... Elements> constexpr auto operator""_hash() { return hash(message<Elements...>{}); }

static constexpr auto Valid = "valid"_msg;
static constexpr auto Invalid = "invalid"_msg;
static constexpr auto Unknown = "unknown"_msg;

class result_status
{
public:
  constexpr result_status() = default;
  constexpr result_status(const result_status& other) = default;
  constexpr result_status(result_status&& other) : message_{other.message_} { other.message_ = Unknown.sv(); }

  template <char... C> constexpr result_status(message<C...> m) : message_{m.c_str()} {}

  constexpr result_status& operator=(const result_status&) = default;
  constexpr result_status& operator=(result_status&&) = default;

  [[nodiscard]] constexpr std::string_view message() const { return message_; }
  [[nodiscard]] constexpr bool valid() const { return message_.data() == Valid.c_str(); }

private:
  std::string_view message_ = Unknown.sv();
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

inline std::size_t hash(const result_status& s) { return hash(s.message()); }

template <char... C> [[nodiscard]] inline bool operator==(const message<C...>& lhs, const result_status& rhs)
{
  return lhs.sv() == rhs.message();
}

template <char... C> [[nodiscard]] inline bool operator==(const result_status& lhs, const message<C...>& rhs)
{
  return lhs.message() == rhs.sv();
}

template <char... C> [[nodiscard]] inline bool operator!=(const message<C...>& lhs, const result_status& rhs)
{
  return !(lhs == rhs);
}

template <char... C> [[nodiscard]] inline bool operator!=(const result_status& lhs, const message<C...>& rhs)
{
  return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& os, const result_status& s) { return os << s.message(); }

template <char... C> inline std::ostream& operator<<(std::ostream& os, const message<C...> m) { return os << m.sv(); }

}  // namespace zen
