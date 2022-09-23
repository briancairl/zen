#pragma once

// C++ Standard Library
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

// Zen
#include <zen/core.hpp>
#include <zen/executor/executor.hpp>
#include <zen/meta/invocable.hpp>

namespace zen::exec
{

template <typename FuncWrapperT = std::function<void()>, typename FuncWrapperAllocatorT = std::allocator<FuncWrapperT>>
class thread_pool final : public executor<thread_pool<FuncWrapperT, FuncWrapperAllocatorT>>
{
  using base = executor<thread_pool>;
  friend base;

public:
  explicit thread_pool(std::size_t worker_count = std::thread::hardware_concurrency()) :
      is_working_{true}, worker_count_{worker_count}
  {
    // Use malloc to defer initialization
    workers_ = reinterpret_cast<std::thread*>(std::malloc(worker_count_ * sizeof(std::thread)));

    // Construct threads in place
    for (std::size_t i = 0; i < worker_count_; ++i)
    {
      new (workers_ + i) std::thread{[this] { work_loop(); }};
    }
  }

  ~thread_pool()
  {
    // Stop workers
    {
      std::lock_guard lock{work_queue_mtx_};
      is_working_ = false;
      work_queue_cv_.notify_all();
    }

    // Join + destroy each worker thread
    for (std::size_t i = 0; i < worker_count_; ++i)
    {
      workers_[i].join();
      workers_[i].~thread();
    }

    // Free thread memory
    std::free(workers_);
  }

  [[nodiscard]] constexpr std::size_t workers() const { return worker_count_; }

private:
  template <typename FnT> constexpr void execute_impl(FnT&& fn)
  {
    std::lock_guard lock{work_queue_mtx_};
    work_queue_.emplace_back(std::forward<FnT>(fn));
    work_queue_cv_.notify_one();
  };

  void work_loop()
  {
    std::unique_lock lock{work_queue_mtx_};
    while (is_working_)
    {
      if (work_queue_.empty())
      {
        work_queue_cv_.wait(lock);
      }
      else
      {
        auto work = std::move(work_queue_.back());
        work_queue_.pop_back();
        lock.unlock();
        work();
        lock.lock();
      }
    }
  }

  std::vector<FuncWrapperT, FuncWrapperAllocatorT> work_queue_;
  std::mutex work_queue_mtx_;
  std::condition_variable work_queue_cv_;

  bool is_working_;
  std::size_t worker_count_;
  std::thread* workers_;
};

class thread_pool_status_handle : public executor_status_handle<thread_pool_status_handle>
{
  friend class executor_status_handle<thread_pool_status_handle>;

public:
  thread_pool_status_handle() : working_{true} {};

private:
  bool is_working_impl() const { return static_cast<bool>(working_); };
  void cancel_impl() { working_ = false; };
  std::atomic<bool> working_;
};

}  // namespace zen::exec

namespace zen
{

template <typename F, typename A, typename... InvocableTs> class any_dispatch<exec::thread_pool<F, A>, InvocableTs...>
{
  static constexpr std::size_t N = sizeof...(InvocableTs);

public:
  explicit constexpr any_dispatch(exec::thread_pool<F, A>& exec, InvocableTs&&... fs) :
      e_{exec}, invocables_{std::forward<InvocableTs>(fs)...}
  {
    static_assert(sizeof...(InvocableTs) > 0);
  };

  template <typename ValueT> [[nodiscard]] decltype(auto) operator()(ValueT&& v) const
  {
    return exec_impl(std::forward<ValueT>(v), std::make_index_sequence<N>{});
  }

private:
  template <typename ValueT, std::size_t... Is> decltype(auto) exec_impl(ValueT&& v, std::index_sequence<Is...>) const
  {
    using result_type = meta::
      result_of_apply_t<meta::first_t<std::tuple<InvocableTs...>>, result_as_tuple_t<std::remove_reference_t<ValueT>>>;

    result_type retval = result_type::error(v.message());
    if (v.valid())
    {
      std::promise<result_type> promises[N];
      std::future<result_type> results[N] = {promises[Is].get_future()...};

      exec::thread_pool_status_handle status_handle;

      // Queue up all work to run simultaneously
      {
        [[maybe_unused]] const auto _ =
          (((e_.execute([&] {
              const auto& fn = std::get<Is>(this->invocables_);
              if constexpr (meta::can_apply_v<
                              decltype(fn),
                              decltype(std::tuple_cat(std::forward_as_tuple(status_handle), v.as_tuple()))>)
              {
                promises[Is].set_value(
                  std::apply(fn, std::tuple_cat(std::forward_as_tuple(status_handle), v.as_tuple())));
              }
              else
              {
                promises[Is].set_value(std::apply(fn, v.as_tuple()));
              }
            })),
            Is) +
           ...);
      }

      // Get result
      {
        [[maybe_unused]] const auto _ =
          ((retval = results[Is].get(), (retval.valid() || (status_handle.cancel(), false))) || ...);
      }
    }
    else
    {
      retval = result_type::error(v.message());
    }
    return retval;
  }

  exec::thread_pool<F, A>& e_;
  std::tuple<InvocableTs&&...> invocables_;
};

template <typename F, typename A, typename... InvocableTs> class all_dispatch<exec::thread_pool<F, A>, InvocableTs...>
{
  static constexpr std::size_t N = sizeof...(InvocableTs);

public:
  explicit constexpr all_dispatch(exec::thread_pool<F, A>& exec, InvocableTs&&... fs) :
      e_{exec}, invocables_{std::forward<InvocableTs>(fs)...}
  {
    static_assert(sizeof...(InvocableTs) > 0);
  };

  template <typename ValueT> [[nodiscard]] decltype(auto) operator()(ValueT&& v) const
  {
    return exec_impl(std::forward<ValueT>(v), std::make_index_sequence<N>{});
  }

private:
  template <typename ValueT, std::size_t... Is> decltype(auto) exec_impl(ValueT&& v, std::index_sequence<Is...>) const
  {
    using result_type = make_result_t<
      std::tuple<InvocableTs...>,
      std::tuple<
        result_as_tuple_t<std::remove_reference_t<ValueT>>,
        meta::
          append_t<std::tuple<exec::thread_pool_status_handle>, result_as_tuple_t<std::remove_reference_t<ValueT>>>>>;

    result_type retval = result_type::error(v.message());
    if (v.valid())
    {
      std::tuple<std::promise<meta::result_of_apply_t<
        InvocableTs,
        result_as_tuple_t<std::remove_reference_t<ValueT>>,
        meta::append_t<
          std::tuple<exec::thread_pool_status_handle>,
          result_as_tuple_t<std::remove_reference_t<ValueT>>>>>...>
        promises;

      std::tuple<std::future<meta::result_of_apply_t<
        InvocableTs,
        result_as_tuple_t<std::remove_reference_t<ValueT>>,
        meta::append_t<
          std::tuple<exec::thread_pool_status_handle>,
          result_as_tuple_t<std::remove_reference_t<ValueT>>>>>...>
        results{std::get<Is>(promises).get_future()...};

      exec::thread_pool_status_handle status_handle;

      // Queue up all work to run simultaneously
      {
        [[maybe_unused]] const auto _ =
          (((e_.execute([&] {
              const auto& fn = std::get<Is>(this->invocables_);
              if constexpr (meta::can_apply_v<
                              decltype(fn),
                              decltype(std::tuple_cat(std::forward_as_tuple(status_handle), v.as_tuple()))>)
              {
                std::get<Is>(promises).set_value(
                  std::apply(fn, std::tuple_cat(std::forward_as_tuple(status_handle), v.as_tuple())));
              }
              else
              {
                std::get<Is>(promises).set_value(std::apply(fn, v.as_tuple()));
              }
            })),
            Is) +
           ...);
      }

      // Get results
      retval = result_type::create(make_deferred([&status_handle, &f = std::get<Is>(results)]() {
        auto result = f.get();
        if (!result.valid())
        {
          status_handle.cancel();
        }
        return result;
      })...);
    }
    else
    {
      retval = result_type::error(v.message());
    }
    return retval;
  }

  exec::thread_pool<F, A>& e_;
  std::tuple<InvocableTs&&...> invocables_;
};

}  // namespace zen