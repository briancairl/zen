#pragma once

// C++ Standard Library
#include <type_traits>
#include <utility>

namespace zen
{

/**
 * @brief Holds a value whose constructor/destructor can be manually invoked
 *
 * This is used to deal with objects which have non-default constructors:
 * - Defers construction of a value of type <code>T</code> to manual call to <code>emplace</code>
 * - Requires manual destruction with <code>destroy</code>
 *
 * @tparam T  value type
 */
template <typename T> class value_mem
{
public:
  constexpr value_mem() = default;

  /**
   * @copydoc emplace
   */
  template <typename... ArgTs> constexpr value_mem(ArgTs&&... args) { emplace(std::forward<ArgTs>(args)...); }

protected:
  /**
   * @brief Returns reference to held value <code>T</code>
   *
   * @warning behavior undefined if <code>emplace</code> has not been called
   */
  [[nodiscard]] constexpr T& operator*() & { return (*data()); }

  /**
   * @brief Returns immutable reference to held value <code>T</code>
   *
   * @warning behavior undefined if <code>emplace</code> has not been called
   */
  [[nodiscard]] constexpr const T& operator*() const& { return (*data()); }

  /**
   * @brief Returns reference to held value <code>T</code>
   *
   * @warning behavior undefined if <code>emplace</code> has not been called
   */
  [[nodiscard]] constexpr T&& operator*() && { return (*data()); }

  /**
   * @brief Returns immutable reference to held value <code>T</code>
   *
   * @warning behavior undefined if <code>emplace</code> has not been called
   */
  [[nodiscard]] constexpr const T&& operator*() const&& { return (*data()); }

  /**
   * @brief Returns pointer to held value <code>T</code>
   *
   * @warning behavior undefined if <code>emplace</code> has not been called
   */
  [[nodiscard]] constexpr T* operator->() { return data(); }

  /**
   * @brief Returns immutable pointer to held value <code>T</code>
   *
   * @warning behavior undefined if <code>emplace</code> has not been called
   */
  [[nodiscard]] constexpr const T* operator->() const { return data(); }

  /**
   * @brief Forwards <code>ArgTs...</code> and invokes constructor associated with <code>T</code>
   */
  template <typename... ArgTs> void emplace(ArgTs&&... args) { new (data()) T{std::forward<ArgTs>(args)...}; }

  /**
   * @brief Invokes constructor associated with <code>T</code>
   *
   * @warning behavior undefined if <code>emplace</code> has not been called
   */
  constexpr void destroy()
  {
    if constexpr (!std::is_trivial<T>())
    {
      data()->~T();
    }
  }

private:
  /**
   * @brief Returns pointer to value_buffer_, interpret as pointer-to-value type <code>T*</code>
   */
  T* data() { return reinterpret_cast<T*>(&value_buffer_); }

  /**
   * @brief Returns pointer to value_buffer_, interpret as pointer-to-value type <code>const T*</code>
   */
  const T* data() const { return reinterpret_cast<const T*>(&value_buffer_); }

  /// Held-value buffer
  alignas(T) std::byte value_buffer_[sizeof(T)];
};

}  // namespace zen
