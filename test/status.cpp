// GTest
#include <gtest/gtest.h>

// Zen
#include <zen/status.hpp>

using namespace zen;

TEST(Status, Default)
{
  status s;
  ASSERT_FALSE(s) << s.message();
  EXPECT_EQ(s, status::unknown);
}

TEST(Status, Valid)
{
  status s{status::valid};
  ASSERT_TRUE(s) << s.message();
  EXPECT_EQ(s, status::valid);
}

TEST(Status, Invalid)
{
  status s{status::invalid};
  ASSERT_FALSE(s) << s.message();
  EXPECT_EQ(s, status::invalid);
}

TEST(Status, InvalidFromCustomMessage)
{
  status s{"something went wrong"_msg};
  ASSERT_FALSE(s) << s.message();
  EXPECT_EQ(s, "something went wrong"_msg);
}

TEST(Status, SwitchCase)
{
  status s{"something went wrong"_msg};

  bool correct = false;
  switch (hash(s))
  {
  case "something but not exactly"_msg: {
    break;
  }
  case "something went wrong"_msg: {
    correct = true;
    break;
  }
  }

  ASSERT_TRUE(correct);
}

TEST(Status, Move)
{
  status s{status::valid};
  ASSERT_TRUE(s) << s.message();

  status s_moved{std::move(s)};
  EXPECT_EQ(s_moved, status::valid) << s_moved.message();
  EXPECT_EQ(s, status::unknown) << s_moved.message();
}
