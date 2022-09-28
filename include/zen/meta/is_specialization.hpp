#pragma once

namespace zen::meta
{

/**
 * @brief Checks if \c T is a specialization of a template \c Tmpl
 */
template <typename T, template <typename...> class Tmpl> struct is_specialization : std::false_type
{};

template <template <typename...> class Tmpl, typename... Ts>
struct is_specialization<Tmpl<Ts...>, Tmpl> : std::true_type
{};

template <typename T, template <typename...> class Tmpl>
static constexpr bool is_specialization_v = is_specialization<T, Tmpl>::value;

}  // namespace zen::meta