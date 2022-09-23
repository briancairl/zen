// GTest
#include <gtest/gtest.h>

// Zen
#include <zen/executor.hpp>

using namespace zen;

TEST(ThreadPool, DefaultEmpty)
{
  exec::thread_pool pool{};
  ASSERT_GT(pool.workers(), 0UL);
}
