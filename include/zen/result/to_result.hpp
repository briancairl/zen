#pragma once

// Zen
#include <zen/fwd.hpp>

namespace zen
{

/**
 * @brief Converts a type \c T to a corresponding result<T>
 */
template <typename T> struct to_result
{
  using type = result<T>;
};

/**
 * @copydoc to_result
 */
template <typename T> struct to_result<result<T>>
{
  using type = result<T>;
};

/**
 * @copydoc to_result
 */
template <typename T> using to_result_t = typename to_result<T>::type;

}  // namespace zen
