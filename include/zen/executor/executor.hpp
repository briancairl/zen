#pragma once

// C++ Standard Library
#include <type_traits>
#include <utility>

namespace zen::exec
{

template <typename ExecutorT> class executor
{
public:
  template <typename FnT> constexpr void execute(FnT&& fn) { derived()->execute_impl(std::forward<FnT>(fn)); };

private:
  [[nodiscard]] constexpr ExecutorT* derived() { return reinterpret_cast<ExecutorT*>(this); }
  [[nodiscard]] constexpr const ExecutorT* derived() const { return reinterpret_cast<const ExecutorT*>(this); }
};

template <typename HandleT> class executor_status_handle
{
public:
  [[nodiscard]] constexpr bool is_working() const { return derived()->is_working_impl(); };
  [[nodiscard]] constexpr bool is_cancelled() const { return !derived()->is_working_impl(); };
  constexpr void cancel() { derived()->cancel_impl(); };
  constexpr void yield() const { derived()->yield_impl(); };

private:
  constexpr static void yield_impl()
  { /*fallback*/
  }
  constexpr HandleT* derived() { return reinterpret_cast<HandleT*>(this); }
  constexpr const HandleT* derived() const { return reinterpret_cast<const HandleT*>(this); }
};

template <typename DerivedT>
[[nodiscard]] constexpr bool executor_status_handle_test(executor_status_handle<DerivedT>* _)
{
  return true;
}
[[nodiscard]] constexpr bool executor_status_handle_test(...) { return false; }

template <typename T>
struct is_executor_status_handle
    : std::integral_constant<bool, executor_status_handle_test(std::add_pointer_t<T>{nullptr})>
{};

template <typename T> static constexpr bool is_executor_status_handle_v = is_executor_status_handle<T>::value;

}  // namespace zen::exec
