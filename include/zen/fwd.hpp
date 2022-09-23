#pragma once

namespace zen
{

template <typename ExecutorT> class executor;

template <typename... Ts> class result;

template <typename T> struct is_result : std::false_type
{};

template <typename... Ts> struct is_result<result<Ts...>> : std::true_type
{};

template <typename T> constexpr bool is_result_v = is_result<T>::value;

template <typename T> struct result_size;

template <typename... Ts> struct result_size<result<Ts...>>
{
  static constexpr std::size_t value = sizeof...(Ts);
};

template <typename T> constexpr bool result_size_v = result_size<T>::value;

template <typename... Ts> class any_dispatch;

template <typename... Ts> class all_dispatch;

}  // namespace zen
