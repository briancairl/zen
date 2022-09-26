#pragma once

// C++ Standard Library
#include <future>
#include <tuple>
#include <utility>

// Zen
#include <zen/core.hpp>
#include <zen/executor/thread_pool.hpp>
#include <zen/meta/invocable.hpp>

namespace zen
{

template <typename F, typename A, typename... InvocableTs> class any_dispatch<exec::thread_pool<F, A>, InvocableTs...>
{
public:
  explicit constexpr any_dispatch(exec::thread_pool<F, A>& exec, InvocableTs&&... fs) :
      e_{exec}, invocables_{std::forward<InvocableTs>(fs)...}
  {
    static_assert(sizeof...(InvocableTs) > 0, "At least one invocable must be specified");
  };

  template <typename... ValueTs> decltype(auto) operator()(ValueTs&&... values) const
  {
    static constexpr std::size_t N = sizeof...(InvocableTs);
    return exec_impl(std::make_index_sequence<N>{}, std::forward<ValueTs>(values)...);
  }

private:
  template <typename... ValueTs, std::size_t... Is>
  decltype(auto) exec_impl(std::index_sequence<Is...> _, ValueTs&&... values) const
  {
    // clang-format off
    using result_type = meta::result_of_apply_t<
      meta::first_t<InvocableTs...>,
      /*overload 1*/std::tuple<ValueTs...>,
      /*overload 2*/std::tuple<exec::thread_pool_handle, ValueTs...>>;

    static_assert(
      (sizeof...(InvocableTs) == 1) ||
        (std::is_same_v<
           result_type,
           meta::result_of_apply_t<
             InvocableTs,
             /*overload 1*/std::tuple<ValueTs...>,
             /*overload 2*/std::tuple<exec::thread_pool_handle, ValueTs...>>> &&
         ...),
      "'InvocableTs' executed under [any_dispatch] must all have the same return type");

    static constexpr std::size_t N = sizeof...(Is);
    std::promise<result_type> promises[N];
    std::future<result_type> results[N] = {promises[Is].get_future()...};

    exec::thread_pool_handle handle;

    // Queue up all work to run simultaneously
    {
      [[maybe_unused]] const auto unused =
        (((e_.execute([&] {
            const auto& fn = std::get<Is>(this->invocables_);
            if constexpr (meta::can_apply_v<decltype(fn), std::tuple<exec::thread_pool_handle, ValueTs...>>)
            {
              promises[Is].set_value(
                std::apply(fn, std::tuple_cat(std::forward_as_tuple(handle), std::forward_as_tuple(values...))));
            }
            else
            {
              promises[Is].set_value(std::apply(fn, std::forward_as_tuple(values...)));
            }
          })),
          Is) +
         ...);
    }

    // Get result
    result_type r;
    {
      [[maybe_unused]] const auto unused = ((r = results[Is].get(), (r.valid() || (handle.cancel(), false))) || ...);
    }

    // clang-format on
    return r;
  }

  exec::thread_pool<F, A>& e_;
  std::tuple<InvocableTs&&...> invocables_;
};

template <typename F, typename A, typename... InvocableTs> class all_dispatch<exec::thread_pool<F, A>, InvocableTs...>
{
public:
  explicit constexpr all_dispatch(exec::thread_pool<F, A>& exec, InvocableTs&&... fs) :
      e_{exec}, invocables_{std::forward<InvocableTs>(fs)...}
  {
    static_assert(sizeof...(InvocableTs) > 0, "At least one invocable must be specified");
  };

  template <typename... ValueTs> decltype(auto) operator()(ValueTs&&... values) const
  {
    static constexpr std::size_t N = sizeof...(InvocableTs);
    return exec_impl(std::make_index_sequence<N>{}, std::forward<ValueTs>(values)...);
  }

private:
  template <typename... ValueTs, std::size_t... Is>
  decltype(auto) exec_impl(std::index_sequence<Is...> _, ValueTs&&... values) const
  {
    // clang-format off
    exec::thread_pool_handle handle;

    // Create promises
    auto ps = std::make_tuple(
      std::promise<
        to_result_t<
          meta::result_of_apply_t<
            decltype(std::get<Is>(invocables_)),
            /*overload 1*/std::tuple<ValueTs...>,
            /*overload 2*/std::tuple<exec::thread_pool_handle, ValueTs...>
          >
        >
      >{}...);

    // Gather futures from promises
    auto fs = std::make_tuple(std::get<Is>(ps).get_future()...);

    // Start work
    {
      [[maybe_unused]] const auto unused = (
        (e_.execute(
        [&handle, &p=std::get<Is>(ps), &fn=std::get<Is>(invocables_), &values...]()
        {
          if constexpr (meta::can_apply_v<decltype(fn), std::tuple<exec::thread_pool_handle, ValueTs...>>)
          {
            p.set_value(fn(handle, std::forward<ValueTs>(values)...));
          }
          else
          {
            p.set_value(fn(std::forward<ValueTs>(values)...));
          }
        }), true) && ...
      );
    }

    // Create result from async functions
    auto r = create(
      make_deferred_result([&handle, &f=std::get<Is>(fs)]() mutable
      {
        auto r = f.get();
        
        if (!r.valid())
        {
          handle.cancel();
        }
        return r;
      })...
    );

    // Block on any remaining work
    {
      [[maybe_unused]] const auto unused = ((!std::get<Is>(fs).valid() || (std::get<Is>(fs).wait(), true)) && ...);
    }

    return r;
    // clang-format on
  }

  exec::thread_pool<F, A>& e_;
  std::tuple<InvocableTs&&...> invocables_;
};

}  // namespace zen