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

template <typename HandleT> class executor_handle
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

template <typename T> struct is_executor_handle : std::integral_constant<bool, std::is_base_of_v<executor_handle<T>, T>>
{};

template <typename T> static constexpr bool is_executor_handle_v = is_executor_handle<T>::value;

}  // namespace zen::exec
