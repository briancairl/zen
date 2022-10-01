#pragma once

// C++ Standard Library
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

// Zen
#include <zen/executor/executor.hpp>

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

class thread_pool_handle : public executor_handle<thread_pool_handle>
{
  friend class executor_handle<thread_pool_handle>;

public:
  thread_pool_handle() : working_{true} {};

private:
  bool is_working_impl() const { return static_cast<bool>(working_); };
  void cancel_impl() { working_ = false; };
  std::atomic<bool> working_;
};

}  // namespace zen::exec
