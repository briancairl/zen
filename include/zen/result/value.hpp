#pragma once

// C++ Standard Library
#include <utility>

namespace zen
{

template <typename T> class result_value
{
public:
  constexpr result_value() = default;
  constexpr result_value(const T& value) { emplace(value); }
  constexpr result_value(T&& value) { emplace(std::move(value)); }

  [[nodiscard]] constexpr T& value() { return (*data()); }
  [[nodiscard]] constexpr const T& value() const { return (*data()); }

  [[nodiscard]] constexpr T& operator*() { return (*data()); }
  [[nodiscard]] constexpr const T& operator*() const { return (*data()); }

  template <typename... ArgTs> void emplace(ArgTs&&... args) { new (data()) T{std::forward<ArgTs>(args)...}; }

protected:
  void destroy() { data()->~T(); }

private:
  T* data() { return reinterpret_cast<T*>(&value_buffer_); }
  const T* data() const { return reinterpret_cast<const T*>(&value_buffer_); }

  alignas(T) std::byte value_buffer_[sizeof(T)];
};

}  // namespace zen
