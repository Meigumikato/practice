#include "task.hpp"
#include "io_context.hpp"

#include <gtest/gtest.h>

int x = 0;

Task<int> Take() {
  co_return 42;
}

Task<int> Sum(int a, int c) { co_return a + c; }

Task<int> Sum4() {
  sleep(1);
  auto x2 = co_await Take();
  auto x1 = co_await Take();
  int sum = co_await Sum(x2, x1);
  x = sum;
  co_return sum;
}

TEST(io_context, execute) {
  IoContext ctx;

  ctx.Execute(Sum4());

  ASSERT_NO_FATAL_FAILURE(ctx.RunOnce()) << "RunOnce Failed";

  ASSERT_EQ(x, 42 + 42) << "Task Not Execute";
}

