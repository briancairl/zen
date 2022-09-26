// GTest
#include <gtest/gtest.h>

// Zen
#include <zen/status.hpp>

using namespace zen;

TEST(Status, Default)
{
  result_status s;
  ASSERT_FALSE(s.valid()) << s.message();
  EXPECT_EQ(s, Unknown);
}

TEST(Status, Valid)
{
  result_status s{Valid};
  ASSERT_TRUE(s.valid()) << s.message();
  EXPECT_EQ(s, Valid);
}

TEST(Status, Invalid)
{
  result_status s{Invalid};
  ASSERT_FALSE(s.valid()) << s.message();
  EXPECT_EQ(s, Invalid);
}

TEST(Status, InvalidFromCustomMessage)
{
  result_status s{"something went wrong"_msg};
  ASSERT_FALSE(s.valid()) << s.message();
  EXPECT_EQ(s, "something went wrong"_msg);
}

TEST(Status, SwitchCase)
{
  result_status s{"something went wrong"_msg};

  bool correct = false;
  switch (hash(s))
  {
  case "something but not exactly"_hash: {
    break;
  }
  case "something went wrong"_hash: {
    correct = true;
    break;
  }
  }

  ASSERT_TRUE(correct);
}

TEST(Status, Move)
{
  result_status s{Valid};
  ASSERT_TRUE(s.valid()) << s.message();

  result_status s_moved{std::move(s)};
  EXPECT_EQ(s_moved, Valid) << s_moved.message();
  EXPECT_EQ(s, Unknown) << s_moved.message();
}
