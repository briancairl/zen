// GTest
#include <gtest/gtest.h>

// Zen
#include <zen/result.hpp>

using namespace zen;

TEST(Result, Default)
{
  result<int> r;
  ASSERT_FALSE(r.valid()) << r.status();
  EXPECT_EQ(r.status(), Unknown);
}

TEST(Result, CreateValidFromDeferredNoArg)
{
  auto r = create(
    make_deferred_result([] { return result<int>{1}; }),
    make_deferred_result([] { return result<int>{2}; }),
    make_deferred_result([] { return result<int>{3}; }));

  ASSERT_TRUE(r.valid()) << r.status();
  EXPECT_EQ(r.value(), std::make_tuple(1, 2, 3));
}

TEST(Result, CreateInvalidFromDeferredNoArg)
{
  bool invoked_last = false;
  auto r = create(
    make_deferred_result([] { return result<int>{1}; }),
    make_deferred_result([]() -> result<int> { return "nope"_msg; }),
    make_deferred_result([&invoked_last] {
      invoked_last = true;
      return result<int>{3};
    }));

  ASSERT_FALSE(r.valid()) << r.status();
  ASSERT_FALSE(invoked_last);
}

TEST(Result, CreateValidFromDeferredWithArg)
{
  auto r = create(
    make_deferred_result([](const auto& r) { return result<int>{r}; }, std::make_tuple(1)),
    make_deferred_result([](const auto& r) { return result<int>{r}; }, std::make_tuple(2)),
    make_deferred_result([](const auto& r) { return result<int>{r}; }, std::make_tuple(3)));

  ASSERT_TRUE(r.valid()) << r.status();
  EXPECT_EQ(r.value(), std::make_tuple(1, 2, 3));
}

TEST(Result, CreateInvalidFromDeferredWithArg)
{
  bool invoked_last = false;
  auto r = create(
    make_deferred_result([](const auto& r) { return result<int>{1}; }, std::make_tuple(1)),
    make_deferred_result([](const auto& r) -> result<int> { return "nope"_msg; }, std::make_tuple(2)),
    make_deferred_result([&invoked_last] {
      invoked_last = true;
      return result<int>{3};
    }));

  ASSERT_FALSE(r.valid()) << r.status();
  ASSERT_FALSE(invoked_last);
}

TEST(Result, CreateMergedResult)
{
  auto r = create(
    make_deferred_result([](const auto& r) { return make_result(1, 2); }, std::make_tuple(1)),
    make_deferred_result([](const auto& r) { return make_result(std::make_tuple(3)); }, std::make_tuple(2)));

  ASSERT_TRUE(r.valid()) << r.status();
  EXPECT_EQ(r.value(), std::make_tuple(1, 2, 3));
}
