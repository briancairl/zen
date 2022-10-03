#pragma once

// C++ Standard Library
#include <utility>

// Zen
#include <zen/fwd.hpp>
#include <zen/meta/invocable.hpp>
#include <zen/meta/transform.hpp>
#include <zen/result/to_result.hpp>

namespace zen
{
#define DOXYGEN_SHOULD_SKIP_THIS 1
#ifdef DOXYGEN_SHOULD_SKIP_THIS
namespace detail
{

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

}  // namespace detail
#endif  // DOXYGEN_SHOULD_SKIP_THIS

/**
 * @brief An invocable packaged with its arguments, which returns a result
 */
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

/**
 * @brief An invocable with no arguments, which returns a result
 */
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

/**
 * @brief Traits type which isolates the type returned by <code>deferred_result::operator()</code>
 */
template <typename D> struct deferred_result_of;

/**
 * @copydoc deferred_result_of
 */
template <typename InvocableT, typename InputT> struct deferred_result_of<deferred_result<InvocableT, InputT>>
{
  using type = typename deferred_result<InvocableT, InputT>::return_type;
};

/**
 * @copydoc deferred_result_of
 */
template <typename D> using deferred_result_of_t = typename deferred_result_of<std::remove_reference_t<D>>::type;

/**
 * @brief Creates a deferred_result with no arguments
 */
template <typename InvocableT> decltype(auto) make_deferred_result(InvocableT&& invocable)
{
  return deferred_result<std::remove_reference_t<InvocableT>, void>{std::forward<InvocableT>(invocable)};
}

/**
 * @brief Creates a deferred_result with a <code>std::tuple</code> of arguments
 */
template <typename InvocableT, typename ArgTupleT>
decltype(auto) make_deferred_result(InvocableT&& invocable, ArgTupleT&& arg)
{
  return deferred_result<std::remove_reference_t<InvocableT>, std::remove_reference_t<ArgTupleT>>{
    std::forward<InvocableT>(invocable), std::forward<ArgTupleT>(arg)};
}

/**
 * @brief Creates a result from a single deferred invocable
 *
 * @param dr  invocable which return a result
 */
template <typename DeferredT> decltype(auto) create(DeferredT&& dr) { return dr(); }

/**
 * @brief Creates a result from two or more deferred invocables
 *
 * Short-circuits when an invalid result is returned by <code>d1 ... dn</code>
 *
 * @param d1  first invocable
 * @param d2  second invocable
 * @param dn...  other invocables
 */
template <typename DeferredT1, typename DeferredT2, typename... OtherDeferredTs>
decltype(auto) create(DeferredT1&& d1, DeferredT2&& d2, OtherDeferredTs&&... dn)
{
  std::
    tuple<deferred_result_of_t<DeferredT1>, deferred_result_of_t<DeferredT2>, deferred_result_of_t<OtherDeferredTs>...>
      results;
  return detail::create(
    results,
    std::forward_as_tuple(
      std::forward<DeferredT1>(d1), std::forward<DeferredT2>(d2), std::forward<OtherDeferredTs>(dn)...),
    std::make_index_sequence<sizeof...(OtherDeferredTs) + 2>{});
}

}  // namespace zen
