#pragma once

namespace zen::meta
{
/**
 * @brief Grabs first type in VariadicPackT template argument list
 */
template <typename VariadicPackT> struct first;

template <template <typename...> class VariadicPackTmpl, typename FirstT, typename... OtherTs>
struct first<VariadicPackTmpl<FirstT, OtherTs...>>
{
  using type = FirstT;
};

template <typename VariadicPackT> using first_t = typename first<VariadicPackT>::type;

}  // namespace zen::meta
