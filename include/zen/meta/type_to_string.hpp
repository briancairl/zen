#pragma once

// C++ Standard Library
#include <cstring>
#include <string_view>

namespace zen::meta
{

/**
 * @brief Returns the full-qualified name of a type as a C-style string
 */
template <typename T> const char* type_to_string()
{
#if defined(__clang__) || defined(__GNUC__)
  static constexpr std::string_view SIGNATURE{__PRETTY_FUNCTION__};
  static constexpr std::string_view TOKEN{"T = "};
  static constexpr std::size_t OFFSET = SIGNATURE.find(TOKEN);
  static char type_to_string_storage[SIGNATURE.size() - TOKEN.size() - OFFSET] = {'\0'};
  if (type_to_string_storage[0] == '\0')
  {
    std::memcpy(
      type_to_string_storage, SIGNATURE.data() + TOKEN.size() + OFFSET, SIGNATURE.size() - TOKEN.size() - OFFSET);
    type_to_string_storage[SIGNATURE.size() - TOKEN.size() - OFFSET - 1] = '\0';
  }
  return type_to_string_storage;
#else  // __PRETTY_FUNCTION__
  return "type_to_string<T> unsupported";
#endif  // __PRETTY_FUNCTION__
}

/**
 * @brief Returns the full-qualified name of a type as a string_view
 */
template <typename T> std::string_view type_to_string_view() { return std::string_view{type_to_string<T>()}; }

}  // namespace zen::meta
