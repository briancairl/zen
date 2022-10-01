#pragma once

// C++ Standard Library
#include <iosfwd>
#include <utility>

// Zen
#include <zen/fwd.hpp>
#include <zen/meta/invocable.hpp>
#include <zen/meta/is_specialization.hpp>
#include <zen/meta/transform.hpp>
#include <zen/result/status.hpp>
#include <zen/utility/value_mem.hpp>

namespace zen
{
template <typename T> struct to_result
{
  using type = result<T>;
};

template <typename T> struct to_result<result<T>>
{
  using type = result<T>;
};

template <typename T> using to_result_t = typename to_result<T>::type;

template <typename InvocableT, typename ArgumentTupleT> class deferred_result
{
public:
  using invocable_return_type = meta::result_of_apply_t<InvocableT, ArgumentTupleT>;
  using return_type = to_result_t<invocable_return_type>;
  constexpr deferred_result(InvocableT&& fn, ArgumentTupleT&& args) :
      fn_{std::forward<InvocableT>(fn)}, args_{std::forward<ArgumentTupleT>(args)}
  {}

  [[nodiscard]] constexpr auto operator()() const -> return_type { return std::apply(fn_, args_); }

private:
  InvocableT&& fn_;
  ArgumentTupleT&& args_;
};


template <typename InvocableT> class deferred_result<InvocableT, void>
{
public:
  using invocable_return_type = meta::result_of_apply_t<InvocableT, std::tuple<>>;
  using return_type = to_result_t<invocable_return_type>;
  constexpr deferred_result(InvocableT&& fn) : fn_{std::forward<InvocableT>(fn)} {}

  [[nodiscard]] constexpr auto operator()() const -> return_type { return fn_(); }

private:
  InvocableT&& fn_;
};

template <typename D> struct deferred_result_of;

template <typename InvocableT, typename InputT> struct deferred_result_of<deferred_result<InvocableT, InputT>>
{
  using type = typename deferred_result<InvocableT, InputT>::return_type;
};

template <typename D> using deferred_result_of_t = typename deferred_result_of<D>::type;

template <typename InvocableT> decltype(auto) make_deferred_result(InvocableT&& invocable)
{
  return deferred_result<std::remove_reference_t<InvocableT>, void>{std::forward<InvocableT>(invocable)};
}

template <typename InvocableT, typename ArgTupleT>
decltype(auto) make_deferred_result(InvocableT&& invocable, ArgTupleT&& arg)
{
  return deferred_result<std::remove_reference_t<InvocableT>, std::remove_reference_t<ArgTupleT>>{
    std::forward<InvocableT>(invocable), std::forward<ArgTupleT>(arg)};
}

template <typename T> class result final : private value_mem<T>
{
public:
  constexpr result() = default;

  template <char... Elements>
  constexpr result(message<Elements...> error_message) : value_mem<T>{}, status_{error_message}
  {
    static_assert(
      !are_messages_equal<message<Elements...>, decltype(Valid)>(),
      "To set a valid result, assign a value, not an error message");
  }

  constexpr result(result_status&& status) : value_mem<T>{}, status_{std::move(status)} {}
  constexpr result(const T& value) : value_mem<T>{value}, status_{Valid} {}
  constexpr result(T&& value) : value_mem<T>{std::move(value)}, status_{Valid} {}

  ~result()
  {
    if (status_.valid())
    {
      result::destroy();
    }
  }

  using value_mem<T>::value;
  using value_mem<T>::operator*;

  [[nodiscard]] constexpr result_status status() const { return status_; }

  [[nodiscard]] constexpr bool valid() const { return status_.valid(); }

  [[nodiscard]] constexpr operator bool() const { return status_.valid(); }

private:
  result_status status_;
};

template <typename T> [[nodiscard]] constexpr decltype(auto) as_tuple(const result<T>& r)
{
  return std::forward_as_tuple(r.value());
}

template <typename... Ts> [[nodiscard]] constexpr decltype(auto) as_tuple(const result<std::tuple<Ts...>>& r)
{
  return r.value();
}

template <typename DeferredT> decltype(auto) create(DeferredT deferred_result) { return deferred_result(); }

template <typename ResultTupleT, typename DeferredTupleT, std::size_t... Is>
decltype(auto) create(ResultTupleT&& result_tuple, DeferredTupleT&& deferred_result_tuple, std::index_sequence<Is...> _)
{
  using concat_result_tuple_type =
    decltype(std::tuple_cat(as_tuple(std::get<Is>(std::forward<ResultTupleT>(result_tuple)))...));
  using result_type = result<meta::transform_t<concat_result_tuple_type, std::remove_reference>>;

  result_type r;
  if (((std::get<Is>(std::forward<ResultTupleT>(result_tuple)) =
          std::get<Is>(std::forward<DeferredTupleT>(deferred_result_tuple))(),
        std::get<Is>(std::forward<ResultTupleT>(result_tuple)).valid()) &&
       ...))
  {
    r = result_type{std::tuple_cat(as_tuple(std::get<Is>(std::forward<ResultTupleT>(result_tuple)))...)};
  }
  else
  {
    [[maybe_unused]] const auto _ =
      ((std::get<Is>(std::forward<ResultTupleT>(result_tuple)).valid() ||
        (r = result_type{std::get<Is>(std::forward<ResultTupleT>(result_tuple)).status()}, false)) &&
       ...);
  }
  return r;
}

template <typename DeferredT1, typename DeferredT2, typename... OtherDeferredTs>
decltype(auto) create(DeferredT1 d1, DeferredT2 d2, OtherDeferredTs... dn)
{
  std::
    tuple<deferred_result_of_t<DeferredT1>, deferred_result_of_t<DeferredT2>, deferred_result_of_t<OtherDeferredTs>...>
      results;
  return create(
    results, std::forward_as_tuple(d1, d2, dn...), std::make_index_sequence<sizeof...(OtherDeferredTs) + 2>{});
}

template <typename ResultT> struct result_as_tuple;

template <typename T> struct result_as_tuple<result<T>>
{
  using type = std::tuple<T>;
};

template <typename... Ts> struct result_as_tuple<result<std::tuple<Ts...>>>
{
  using type = std::tuple<Ts...>;
};

template <typename ResultT> using result_as_tuple_t = typename result_as_tuple<ResultT>::type;

template <typename... Ts> decltype(auto) make_result(Ts&&... values)
{
  if constexpr (sizeof...(Ts) > 1)
  {
    return result<std::tuple<std::remove_reference_t<Ts>...>>{{std::forward<Ts>(values)...}};
  }
  else if constexpr (meta::is_specialization_v<std::remove_reference_t<Ts>..., std::tuple>)
  {
    return result<std::remove_reference_t<Ts>...>{std::forward<Ts>(values)...};
  }
  else
  {
    return result<std::remove_reference_t<Ts>...>{std::forward<Ts>(values)...};
  }
}

template <typename T> inline std::ostream& operator<<(std::ostream& os, const result<T>& r)
{
  if (r.valid())
  {
    return os << r.value();
  }
  return os << r.status();
}

}  // namespace zen
