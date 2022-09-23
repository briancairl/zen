// C++ Standard Library
#include <tuple>
#include <type_traits>

// GTest
#include <gtest/gtest.h>

// Zen
#include <zen/meta.hpp>

using namespace zen::meta;

TEST(CanApply, Success)
{
  auto l = [](const int, const float) {};

  EXPECT_TRUE((can_apply_v<decltype(l), std::tuple<int, float>>));
  EXPECT_TRUE((can_apply_v<decltype(l), std::tuple<const int&, const float&>>));
}

TEST(CanApply, Failure)
{
  auto l = [](const int, const float) {};

  EXPECT_FALSE((can_apply_v<decltype(l), std::tuple<float*, int>>));
  EXPECT_FALSE((can_apply_v<decltype(l), std::tuple<const int*, const float&>>));
}

TEST(ResultOfApply, NoAlternatives)
{
  auto l = [](const int, const float) { return double{1}; };

  using result_type = result_of_apply_t<decltype(l), std::tuple<int, float>>;

  EXPECT_TRUE((std::is_same_v<result_type, double>));
}

TEST(ResultOfApply, WithAlternatives)
{
  auto l = [](const int, const float) { return double{1}; };

  using result_type = result_of_apply_t<decltype(l), std::tuple<int*, float*>, std::tuple<int, float>>;

  EXPECT_TRUE((std::is_same_v<result_type, double>));
}

TEST(ResultOfApplyTup, NoAlternatives)
{
  auto l = [](const int, const float) { return double{1}; };

  using result_type = result_of_apply_tup_t<decltype(l), std::tuple<std::tuple<int, float>>>;

  EXPECT_TRUE((std::is_same_v<result_type, double>));
}

TEST(ResultOfApplyTup, WithAlternatives)
{
  auto l = [](const int, const float) { return double{1}; };

  using result_type = result_of_apply_tup_t<decltype(l), std::tuple<std::tuple<int*, float*>, std::tuple<int, float>>>;

  EXPECT_TRUE((std::is_same_v<result_type, double>));
}

TEST(Append, Single)
{
  using lhs_type = std::tuple<const int, const float>;
  using rhs_type = double;
  using expected_type = std::tuple<const int, const float, double>;

  EXPECT_TRUE((std::is_same_v<append_t<lhs_type, rhs_type>, expected_type>))
    << type_to_string_view<lhs_type>() << " + " << type_to_string_view<rhs_type>() << " -/-> "
    << type_to_string_view<expected_type>();
}

TEST(Append, AnotherPack)
{
  using lhs_type = std::tuple<const int, const float>;
  using rhs_type = std::tuple<char, double>;
  using expected_type = std::tuple<const int, const float, char, double>;

  EXPECT_TRUE((std::is_same_v<append_t<lhs_type, rhs_type>, expected_type>))
    << type_to_string_view<lhs_type>() << " + " << type_to_string_view<rhs_type>() << " -/-> "
    << type_to_string_view<expected_type>();
}
