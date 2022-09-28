// C++ Standard Library
#include <vector>

// GTest
#include <gtest/gtest.h>

// Zen
#include <zen/meta.hpp>
#include <zen/zen.hpp>

using namespace zen;

namespace
{

result<int> test_valid_fn1(const int v) { return v + v; }
result<int> test_valid_fn2(const int a, const int b) { return a + b; }

result<int> test_invalid_fn1(const int v) { return "this is an error 1"_msg; }
result<int> test_invalid_fn2(const int a, const int b) { return "this is an error 2"_msg; }

}  // namespace

TEST(Core, Sequence)
{
  // clang-format off
  auto r = pass(1)
         | test_valid_fn1
         | test_valid_fn1;
  // clang-format on

  ASSERT_TRUE(r.valid()) << r.status();
  EXPECT_EQ(*r, 4) << r.status();
}

TEST(Core, SequenceShortCircuited)
{
  bool last_invoked = false;

  // clang-format off
  auto r = pass(1)
         | [](const int a) -> result<int> { return "no"_msg; }
         | [&last_invoked](const int a) -> result<int> { last_invoked = true; return a; };
  // clang-format on

  ASSERT_FALSE(r.valid()) << r.status();
  EXPECT_FALSE(last_invoked) << r.status();
}

TEST(Core, SequenceNonTrivial)
{
  // clang-format off
  auto r = pass(1)
         | [](const auto& a) { return std::vector<int>{a, a, a, a}; }
         | [](auto&& v) { return v.size(); };
  // clang-format on

  ASSERT_TRUE(r.valid()) << r.status();
  EXPECT_EQ(*r, 4UL) << r.status();
}

TEST(Core, AnySuccess)
{
  // clang-format off
  auto r = test_valid_fn1(1)
         | any(test_valid_fn1, test_invalid_fn1);
  // clang-format on

  ASSERT_TRUE(r.valid()) << r.status();
  EXPECT_EQ(*r, 4) << r.status();
}

TEST(Core, AnyFailure)
{
  // clang-format off
  auto r = test_valid_fn1(1)
         | any(test_invalid_fn1, test_invalid_fn1);
  // clang-format on

  ASSERT_FALSE(r.valid()) << r.status();
}

TEST(Core, AnyFailureShortCircuit)
{
  bool last_invoked = false;

  // clang-format off
  auto r = pass(1)
         | any(test_invalid_fn1, test_invalid_fn1)
         | [&last_invoked](const int a) { last_invoked = true; return pass(1, 2, 3); }
         | [&last_invoked](const int a, const int b, const int c) -> result<int> { last_invoked = true; return a; };
  // clang-format on

  ASSERT_FALSE(r.valid()) << r.status();
  EXPECT_FALSE(last_invoked);
}

TEST(Core, AnySuccessMultiInput)
{
  // clang-format off
  auto r = pass(1, 2)
         | any(test_valid_fn2, test_invalid_fn2);
  // clang-format on

  ASSERT_TRUE(r.valid()) << r.status();
  EXPECT_EQ(*r, 3) << r.status();
}

TEST(Core, AnyFailureMultiInput)
{
  // clang-format off
  auto r = pass(1, 2)
         | any(test_invalid_fn2, test_invalid_fn2);
  // clang-format on

  ASSERT_FALSE(r.valid()) << r.status();
}

TEST(Core, AllSuccess)
{
  // clang-format off
  auto r = pass(1)
         | all([](const int a) -> result<int> { return 4; }, test_valid_fn1, [](const int a) { return pass(1, 2, 3); });
  // clang-format on

  ASSERT_TRUE(r.valid()) << r.status();
  EXPECT_EQ(*r, std::make_tuple(4, 2, 1, 2, 3)) << r.status();
}

TEST(Core, AllFailure)
{
  // clang-format off
  auto r = pass(1)
         | all(test_invalid_fn1, test_valid_fn1);
  // clang-format on

  ASSERT_FALSE(r.valid()) << r.status();
}

TEST(Core, AllFailureShortCircuit)
{
  // clang-format off
  auto r = pass(1)
         | all(test_valid_fn1, test_valid_fn1)
         | all(test_invalid_fn2, test_valid_fn2);
  // clang-format on

  ASSERT_FALSE(r.valid()) << r.status();
}


TEST(Parallel, ThreadPoolAnySuccess)
{
  exec::thread_pool tp{4};

  // clang-format off
  auto r = pass(1)
         | any(tp, test_valid_fn1, test_invalid_fn1);
  // clang-format on

  ASSERT_TRUE(r.valid()) << r.status();
  EXPECT_EQ(*r, 2) << r.status();
}

TEST(Parallel, ThreadPoolAnyFailure)
{
  exec::thread_pool tp{4};

  // clang-format off
  auto r = pass(1)
         | any(tp, test_invalid_fn1, [](const auto& h, const int v) -> result<int> { return "no"_msg; });
  // clang-format on

  ASSERT_FALSE(r.valid()) << r.status();
}

TEST(Parallel, ThreadPoolAllSuccess)
{
  exec::thread_pool tp{4};

  // clang-format off
  auto r = test_valid_fn1(1)
         | all(tp, test_valid_fn1, test_valid_fn1);
  // clang-format on

  ASSERT_TRUE(r.valid()) << r.status();

  const auto [a, b] = *r;

  EXPECT_EQ(a, 4) << r.status();
  EXPECT_EQ(b, 4) << r.status();
}

TEST(Parallel, ThreadPoolAllFailure)
{
  exec::thread_pool tp{4};

  // clang-format off
  auto r = test_valid_fn1(1)
         | all(
             tp,
             [](const auto& h, const int) -> result<int> { return 1; },
             [](const int) -> result<float> { return 3.f; },
             test_invalid_fn1);
  // clang-format on

  ASSERT_FALSE(r.valid()) << r.status();
}