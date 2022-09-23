#pragma once

// Zen
#include <zen/meta/append.hpp>

namespace zen
{

template <typename OutputPackT, typename T> struct collect_customization
{
  using type = meta::append_t<OutputPackT, T>;
};

}  // namespace zen

namespace zen::meta
{
namespace detail
{

template <typename OutputPackT, typename InputPackT> struct collect_impl;

template <typename OutputPackT, template <typename...> class PackTmpl, typename FirstT, typename... OtherTs>
struct collect_impl<OutputPackT, PackTmpl<FirstT, OtherTs...>>
    : collect_impl<typename collect_customization<OutputPackT, FirstT>::type, PackTmpl<OtherTs...>>
{};

template <typename OutputPackT, template <typename...> class PackTmpl> struct collect_impl<OutputPackT, PackTmpl<>>
{
  using type = OutputPackT;
};

template <typename PackT> struct empty_pack;

template <template <typename...> class PackTmpl, typename... Ts> struct empty_pack<PackTmpl<Ts...>>
{
  using type = PackTmpl<>;
};

}  // namespace detail

/**
 * @brief Produces a template with a set of collect variadic arguments from a single instance of a variadic template
 */
template <typename PackT> struct collect
{
  using type = typename detail::collect_impl<typename detail::empty_pack<PackT>::type, PackT>::type;
};

}  // namespace zen::meta
