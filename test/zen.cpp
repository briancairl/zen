// GTest
#include <gtest/gtest.h>

// Zen
#include <zen/meta.hpp>
#include <zen/zen.hpp>

using namespace zen;

static result<int> test_valid_fn1(const int v) { return v + v; }
static result<int> test_valid_fn2(const int a, const int b) { return a + b; }

static result<int> test_invalid_fn1(const int v) { return status{"this is an error 1"}; }
static result<int> test_invalid_fn2(const int a, const int b) { return status{"this is an error 2"}; }

TEST(Core, Sequence)
{
  auto r = test_valid_fn1(1) | test_valid_fn1 | test_valid_fn1;

  ASSERT_TRUE(r) << r.message();
  EXPECT_EQ(*r, 8) << r.message();
}

TEST(Core, AnySuccess)
{
  auto r = test_valid_fn1(1) | any(test_valid_fn1, test_invalid_fn1);

  ASSERT_TRUE(r) << r.message();
  EXPECT_EQ(*r, 2) << r.message();
}

TEST(Core, AnyFailure)
{
  auto r = test_valid_fn1(1) | any(test_invalid_fn1, test_invalid_fn1);

  ASSERT_FALSE(r) << r.message();
}

TEST(Core, AnyFailureShortCircuit)
{
  auto r = test_valid_fn1(1) | any(test_invalid_fn1, test_invalid_fn1) | any(test_valid_fn1, test_valid_fn1);

  ASSERT_FALSE(r) << r.message();
}

TEST(Core, AllSuccess)
{
  auto r = test_valid_fn1(1) | all([](const int a) -> result<int> { return 4; }, test_valid_fn1);

  ASSERT_TRUE(r) << r.message();
  EXPECT_EQ((std::get<1>(*r)), 4) << r.message();
}

TEST(Core, AllFailure)
{
  auto r = test_valid_fn1(1) | all(test_invalid_fn1, test_valid_fn1);

  ASSERT_FALSE(r) << r.message();
}

TEST(Core, AllFailureShortCircuit)
{
  auto r = test_valid_fn1(1) | all(test_valid_fn1, test_valid_fn1) | all(test_invalid_fn2, test_valid_fn2);

  ASSERT_FALSE(r) << r.message();
}


// thread pool

TEST(ThreadPool, AnySuccess)
{
  exec::thread_pool tp{4};

  auto r = test_valid_fn1(1) | any(tp, test_valid_fn1, test_invalid_fn1);

  ASSERT_TRUE(r) << r.message();
  EXPECT_EQ(*r, 4) << r.message();
}

TEST(ThreadPool, AnyFailure)
{
  exec::thread_pool tp{4};

  auto r = test_valid_fn1(1) | any(tp, test_invalid_fn1, [](const int v) { return status{"no"}; });

  ASSERT_FALSE(r) << r.message();
}

TEST(ThreadPool, AllSuccess)
{
  exec::thread_pool tp{4};

  auto r = test_valid_fn1(1) | all(tp, test_valid_fn1, test_valid_fn1);

  ASSERT_TRUE(r) << r.message();

  const auto [a, b] = *r;

  EXPECT_EQ(a, 4) << r.message();
  EXPECT_EQ(b, 4) << r.message();
}

TEST(ThreadPool, AllFailure)
{
  exec::thread_pool tp{4};

  auto r = test_valid_fn1(1) |
    all(
             tp,
             [](const auto& h, const int) -> result<int> { return 1; },
             [](const int) -> result<float> { return 3.f; },
             test_invalid_fn1);

  ASSERT_FALSE(r) << r.message();
}