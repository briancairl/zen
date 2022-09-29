#pragma once

// C++ Standard Library
#include <iosfwd>
#include <string_view>
#include <utility>

// Zen
#include <zen/result/message.hpp>

namespace zen
{

template <typename T, T... Elements> constexpr auto operator""_fail() { return message<Elements...>{}; }

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

}  // namespace zen
