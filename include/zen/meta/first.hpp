#pragma once

namespace zen::meta
{
/**
 * @brief Grabs first type in VariadicPackT template argument list
 */
template <typename... Ts> struct first;

template <typename FirstT, typename... OtherTs> struct first<FirstT, OtherTs...>
{
  using type = FirstT;
};

template <typename... Ts> using first_t = typename first<Ts...>::type;

}  // namespace zen::meta
