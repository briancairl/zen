#pragma once

// C++ Standard Library
#include <utility>

// Zen
#include <zen/fwd.hpp>
#include <zen/meta/first.hpp>
#include <zen/result.hpp>

namespace zen
{

template <typename... InvocableTs> class any_dispatch
{
  static constexpr std::size_t N = sizeof...(InvocableTs);

public:
  explicit constexpr any_dispatch(InvocableTs&&... fs) : invocables_{std::forward<InvocableTs>(fs)...} {};

  template <typename ValueT> decltype(auto) operator()(ValueT&& v) const
  {
    return exec_impl(std::forward<ValueT>(v), std::make_index_sequence<N>{});
  }

private:
  template <typename ValueT, std::size_t... Is> decltype(auto) exec_impl(ValueT&& v, std::index_sequence<Is...>) const
  {
    using result_type = meta::
      result_of_apply_t<meta::first_t<std::tuple<InvocableTs...>>, result_as_tuple_t<std::remove_reference_t<ValueT>>>;

    result_type return_value{status{v.message()}};
    v.valid() && ((return_value = std::get<Is>(this->invocables_)(std::forward<ValueT>(v)), return_value) || ...);
    return return_value;
  }

  std::tuple<InvocableTs&&...> invocables_;
};

template <typename... InvocableTs> class all_dispatch
{
  static constexpr std::size_t N = sizeof...(InvocableTs);

public:
  explicit constexpr all_dispatch(InvocableTs&&... fs) : invocables_{std::forward<InvocableTs>(fs)...} {};

  template <typename ValueT> decltype(auto) operator()(ValueT&& v) const
  {
    return exec_impl(std::forward<ValueT>(v), std::make_index_sequence<N>{});
  }

private:
  template <typename ValueT, std::size_t... Is> decltype(auto) exec_impl(ValueT&& v, std::index_sequence<Is...>) const
  {
    using result_type =
      make_result_t<std::tuple<InvocableTs...>, std::tuple<result_as_tuple_t<std::remove_reference_t<ValueT>>>>;

    return v.valid() ? result_type::create(deferred<InvocableTs, std::remove_reference_t<ValueT>>{
                         std::forward<InvocableTs>(std::get<Is>(invocables_)), std::forward<ValueT>(v)}...)
                     : result_type::error(v.message());
  }

  std::tuple<InvocableTs&&...> invocables_;
};

template <typename... Ts> constexpr decltype(auto) begin(Ts&&... t)
{
  return result<std::remove_reference_t<Ts>...>{std::forward<Ts>(t)...};
}

template <typename... Ts> constexpr decltype(auto) any(Ts&&... t)
{
  return any_dispatch<std::remove_reference_t<Ts>...>{std::forward<Ts>(t)...};
}

template <typename... Ts> constexpr decltype(auto) all(Ts&&... t)
{
  return all_dispatch<std::remove_reference_t<Ts>...>{std::forward<Ts>(t)...};
}

template <typename... Ts, typename... InvocableTs>
decltype(auto) operator|(result<Ts...>&& r, any_dispatch<InvocableTs...> dispatch)
{
  return dispatch(r);
}

template <typename... Ts, typename... InvocableTs>
decltype(auto) operator|(result<Ts...>&& r, all_dispatch<InvocableTs...> dispatch)
{
  return dispatch(r);
}

template <typename... Ts, typename Fn> decltype(auto) operator|(result<Ts...>&& r, Fn f)
{
  using return_type = decltype(std::apply(f, r.as_tuple()));
  return r.valid() ? std::apply(f, r.as_tuple()) : return_type::error(r.message());
}

}  // namespace zen
