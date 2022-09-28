#pragma once

// C++ Standard Library
#include <utility>

namespace zen
{

template <typename T> class result;

template <typename... Ts> class any_dispatch;

template <typename... Ts> class all_dispatch;

}  // namespace zen

namespace zen::exec
{

template <typename T> class executor;
template <typename T> class executor_handle;

}  // namespace zen::exec
