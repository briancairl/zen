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
#include <zen/utility/value_mem.hpp>

namespace zen::exec
{

/**
 * @brief Thread pool with variable number of worker threads
 */
template <typename FuncWrapperT = std::function<void()>, typename FuncWrapperAllocatorT = std::allocator<FuncWrapperT>>
class thread_pool final : public executor<thread_pool<FuncWrapperT, FuncWrapperAllocatorT>>
{
  using base = executor<thread_pool>;
  friend base;

  /**
   * @brief <code>std::thread</code> wrapper with deferred startup and join on destroy
   */
  struct deferred_thread_type : value_mem<std::thread>
  {
    template <typename FnT> void start(FnT&& work_loop) { this->emplace(std::forward<FnT>(work_loop)); }

    ~deferred_thread_type()
    {
      (*this)->join();
      this->destroy();
    }
  };

public:
  /**
   * @brief Creates thread_pool with a fixed number of worker threads
   *
   * @param worker_count  number of worker threads; by default, set to the number of hardware cores
   */
  explicit thread_pool(std::size_t worker_count = std::thread::hardware_concurrency()) :
      is_working_{true}, worker_count_{worker_count}, workers_{std::make_unique<deferred_thread_type[]>(worker_count_)}
  {
    // Start thread workloops
    for (std::size_t i = 0; i < worker_count_; ++i)
    {
      workers_[i].start([this] { work_loop(); });
    }
  }

  /**
   * @brief Stops active workers
   */
  ~thread_pool()
  {
    // Stop workers
    std::lock_guard lock{work_queue_mtx_};
    is_working_ = false;
    work_queue_cv_.notify_all();
  }

  /**
   * @brief Returns the number of worker threads
   */
  [[nodiscard]] constexpr std::size_t workers() const { return worker_count_; }

private:
  /**
   * @brief Work-enqueue implementation
   */
  template <typename FnT> constexpr void execute_impl(FnT&& fn)
  {
    std::lock_guard lock{work_queue_mtx_};
    work_queue_.emplace_back(std::forward<FnT>(fn));
    work_queue_cv_.notify_one();
  };

  /**
   * @brief Executes any new work
   */
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
        // Grab next work under lock and remove from queue
        auto work = std::move(work_queue_.back());
        work_queue_.pop_back();

        // Unlock before executing work to allow new work to be
        // enqueue during work execution
        lock.unlock();
        {
          // Do the work
          work();
        }
        lock.lock();
      }
    }
  }

  /// Mutex which synchronizes work_queue_ and is_working_ between threads of execution
  std::mutex work_queue_mtx_;

  /// Queue of work to execute
  std::vector<FuncWrapperT, FuncWrapperAllocatorT> work_queue_;

  /// Conditional variable used to notify about new work
  std::condition_variable work_queue_cv_;

  /// Flag used to indicate that pool is still active
  bool is_working_;

  /// Number of active workers
  std::size_t worker_count_;

  /// Worker threads
  std::unique_ptr<deferred_thread_type[]> workers_;
};

/**
 * @brief Handle used to check if thread pool is still running a given set of work
 */
class thread_pool_handle : public executor_handle<thread_pool_handle>
{
  friend class executor_handle<thread_pool_handle>;

public:
  thread_pool_handle() = default;

private:
  /// @copydoc executor_handle<thread_pool_handle>::is_working_impl
  bool is_working_impl() const { return static_cast<bool>(working_); };

  /// @copydoc executor_handle<thread_pool_handle>::cancel_impl
  void cancel_impl() { working_ = false; };

  /// Atomic flag shared between work to check if executor is still active
  std::atomic<bool> working_{true};
};

}  // namespace zen::exec
