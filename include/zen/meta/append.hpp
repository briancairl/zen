#pragma once

namespace zen::meta
{
/**
 * @brief Appends AppendT to a template with a variadic parameter list
 */
template <typename VariadicPackT, typename AppendT> struct append;

template <template <typename...> class VariadicPackTmpl, typename AppendT, typename... OriginalTs>
struct append<VariadicPackTmpl<OriginalTs...>, AppendT>
{
  using type = VariadicPackTmpl<OriginalTs..., AppendT>;
};

template <template <typename...> class VariadicPackTmpl, typename... AppendTs, typename... OriginalTs>
struct append<VariadicPackTmpl<OriginalTs...>, VariadicPackTmpl<AppendTs...>>
{
  using type = VariadicPackTmpl<OriginalTs..., AppendTs...>;
};

template <typename VariadicPackT, typename AppendT> using append_t = typename append<VariadicPackT, AppendT>::type;

}  // namespace zen::meta
