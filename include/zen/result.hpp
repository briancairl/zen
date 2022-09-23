#pragma once

// C++ Standard Library
#include <utility>

// Zen
#include <zen/fwd.hpp>
#include <zen/meta/collect.hpp>
#include <zen/meta/invocable.hpp>
#include <zen/meta/type_to_string.hpp>

namespace zen
{

template <typename InvocableT, typename ArgTs = void> class deferred;

template <typename InvocableT> class deferred<InvocableT, void>
{
public:
  using original_return_type = meta::result_of_apply_t<InvocableT, std::tuple<>>;
  using return_type =
    std::conditional_t<is_result_v<original_return_type>, original_return_type, result<original_return_type>>;

  constexpr deferred(InvocableT&& fn) : fn_{std::forward<InvocableT>(fn)} {}

  constexpr return_type operator()() const { return fn_(); }

private:
  InvocableT&& fn_;
};

template <typename InvocableT, typename... Ts> class deferred<InvocableT, result<Ts...>>
{
public:
  using original_return_type = meta::result_of_apply_t<InvocableT, std::tuple<Ts...>>;
  using return_type =
    std::conditional_t<is_result_v<original_return_type>, original_return_type, result<original_return_type>>;

  constexpr deferred(InvocableT&& fn, result<Ts...> r) : fn_{std::forward<InvocableT>(fn)}, args_(r.value()) {}

  constexpr return_type operator()() const { return std::apply(fn_, args_); }

private:
  InvocableT&& fn_;
  std::tuple<Ts&...> args_;
};

template <typename InvocableT> decltype(auto) make_deferred(InvocableT&& invocable)
{
  return deferred<std::remove_reference_t<InvocableT>, void>{std::forward<InvocableT>(invocable)};
}

template <typename InvocableT, typename ArgT> decltype(auto) make_deferred(InvocableT&& invocable, ArgT&& arg)
{
  return deferred<std::remove_reference_t<InvocableT>, std::remove_reference_t<ArgT>>{
    std::forward<InvocableT>(invocable), std::forward<ArgT>(arg)};
}

class status
{
  static constexpr const char* kStrMoved{"moved"};
  static constexpr const char* kStrValid{""};

public:
  constexpr status(const status& other) = default;

  constexpr status(status&& other) : message_{other.message_} { other.message_ = kStrMoved; }

  constexpr status& operator=(const status&) = default;
  constexpr status& operator=(status&&) = default;

  constexpr explicit status(const char* reason) : message_{reason} {}
  constexpr const char* message() const { return message_; }

  constexpr bool valid() const { return message_ == kStrValid; }
  constexpr operator bool() const { return valid(); }

protected:
  constexpr status() = default;

private:
  const char* message_ = kStrValid;
};

template <typename T> class mem
{
public:
  constexpr mem() = default;
  constexpr mem(const T& value) { emplace(value); }
  constexpr mem(T&& value) { emplace(std::move(value)); }

  [[nodiscard]] constexpr T& value() { return (*data()); }
  [[nodiscard]] constexpr const T& value() const { return (*data()); }

  [[nodiscard]] constexpr T& operator*() { return (*data()); }
  [[nodiscard]] constexpr const T& operator*() const { return (*data()); }

  template <typename... ArgTs> void emplace(ArgTs&&... args) { new (data()) T{std::forward<ArgTs>(args)...}; }

private:
  T* data() { return reinterpret_cast<T*>(&value_buffer_); }
  const T* data() const { return reinterpret_cast<const T*>(&value_buffer_); }
  void destroy() { data()->~T(); }

  alignas(T) std::byte value_buffer_[sizeof(T)];

  template <typename... Ts> friend class result;
};

template <typename T> class result<T> final : public status, public mem<T>
{
public:
  constexpr result() = default;
  constexpr result(status&& s) : status{std::move(s)} {}
  constexpr result(const T& value) : status{}, mem<T>{value} {}
  constexpr result(T&& value) : status{}, mem<T>{std::move(value)} {}

  ~result()
  {
    if constexpr (std::is_trivial_v<T>)
    {
      return;
    }
    else if (status::valid())
    {
      result::destroy();
    }
  }

  [[nodiscard]] constexpr decltype(auto) as_tuple() { return std::forward_as_tuple(this->value()); }
  [[nodiscard]] constexpr decltype(auto) as_tuple() const { return std::forward_as_tuple(this->value()); }

  template <typename InvocableT, typename... ArgTs>
  [[nodiscard]] static result<T> create(deferred<InvocableT, ArgTs...> creator)
  {
    return creator();
  }

  using mem<T>::operator*;

  static constexpr result<T> error(const char* message) { return result<T>{status{message}}; };
};

template <typename... Ts> class result final : public status
{
public:
  result() = default;
  constexpr result(status&& s) : status{std::move(s)} {}
  constexpr result(const Ts&... values) : status{}, values_{values...} {}
  constexpr result(Ts&&... values) : status{}, values_{std::move(values)...} {}

  ~result()
  {
    if (status::valid())
    {
      destroy_impl(values_, std::make_index_sequence<sizeof...(Ts)>{});
    }
  }

  template <typename... DeferredTs> [[nodiscard]] static result<Ts...> create(DeferredTs&&... deferred)
  {
    static constexpr std::size_t N = sizeof...(Ts);
    static_assert(N == sizeof...(DeferredTs));
    return create_impl(std::forward_as_tuple(deferred...), std::make_index_sequence<N>{});
  }

  [[nodiscard]] constexpr decltype(auto) value()
  {
    return value_impl(values_, std::make_index_sequence<sizeof...(Ts)>{});
  }
  [[nodiscard]] constexpr decltype(auto) value() const
  {
    return value_impl(values_, std::make_index_sequence<sizeof...(Ts)>{});
  }

  [[nodiscard]] constexpr decltype(auto) as_tuple() { return value(); }
  [[nodiscard]] constexpr decltype(auto) as_tuple() const { return value(); }

  [[nodiscard]] constexpr decltype(auto) operator*() { return value(); }
  [[nodiscard]] constexpr decltype(auto) operator*() const { return value(); }

  static constexpr result<Ts...> error(const char* message) { return result<Ts...>{status{message}}; };

private:
  template <typename DeferredTupleT, std::size_t... Is>
  static result<Ts...> create_impl(DeferredTupleT&& deferred, std::index_sequence<Is...>)
  {
    result<Ts...> r;
    std::tuple<result<Ts>...> deferred_results;
    if (((std::get<Is>(deferred_results) = std::get<Is>(deferred)(), std::get<Is>(deferred_results).valid()) && ...))
    {
      [[maybe_unused]] const auto _ =
        ((std::get<Is>(r.values_) = std::move(std::get<Is>(deferred_results)), true) && ...);
    }
    else
    {
      [[maybe_unused]] const auto _ =
        ((std::get<Is>(deferred_results).valid() ||
          (r = result<Ts...>{status{std::get<Is>(deferred_results).message()}}, false)) &&
         ...);
    }
    return r;
  }

  template <typename Tup, std::size_t... Is> static decltype(auto) value_impl(Tup&& tup, std::index_sequence<Is...>)
  {
    return std::forward_as_tuple(std::get<Is>(std::forward<Tup>(tup)).value()...);
  }

  template <typename Tup, std::size_t... Is> static void destroy_impl(Tup&& tup, std::index_sequence<Is...>)
  {
    [[maybe_unused]] const auto _ = (([&] { std::get<Is>(std::forward<Tup>(tup)).destroy(); }(), Is) + ...);
  }

  std::tuple<mem<Ts>...> values_;
};

template <typename OutputPackT, typename... Ts> struct collect_customization<OutputPackT, std::tuple<Ts...>>
{
  using type = meta::append_t<OutputPackT, result<Ts...>>;
};

template <typename OutputPackT, typename... Ts> struct collect_customization<OutputPackT, result<Ts...>>
{
  using type = meta::append_t<OutputPackT, result<Ts...>>;
};

template <typename Invocables, typename Alternatives> struct make_result;

template <typename Alternatives, typename... Invocables> struct make_result<std::tuple<Invocables...>, Alternatives>
{
  using type = typename meta::collect<
    result<meta::result_of_apply_tup_t<std::remove_reference_t<Invocables>, Alternatives>...>>::type;
};

template <typename Invocables, typename Alternatives>
using make_result_t = typename make_result<Invocables, Alternatives>::type;

template <typename ResulT> struct result_as_tuple;

template <typename... Ts> struct result_as_tuple<result<Ts...>>
{
  using type = std::tuple<Ts...>;
};

template <typename ResulT> using result_as_tuple_t = typename result_as_tuple<ResulT>::type;

}  // namespace zen
