#pragma once

// C++ Standard Library
#include <tuple>
#include <type_traits>

namespace zen::meta
{

template <typename Fn, typename Tup> struct can_apply;

template <typename Fn, typename... ArgTs> struct can_apply<Fn, std::tuple<ArgTs...>> : std::is_invocable<Fn, ArgTs...>
{};

template <typename Fn, typename Tup> static constexpr bool can_apply_v = can_apply<Fn, Tup>::value;

template <typename Fn, typename... AltTups> struct result_of_apply
{};

template <typename Fn, typename... ArgTs, typename... OthersTups>
struct result_of_apply<Fn, std::tuple<ArgTs...>, OthersTups...> : std::conditional_t<
                                                                    can_apply_v<Fn, std::tuple<ArgTs...>>,
                                                                    std::invoke_result<Fn, ArgTs...>,
                                                                    result_of_apply<Fn, OthersTups...>>
{};

template <typename Fn, typename... AltTups> using result_of_apply_t = typename result_of_apply<Fn, AltTups...>::type;

}  // namespace zen::meta
