#include "co_spawn.hpp"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include "task.hpp"
#include "io_context.hpp"

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

TEST(co_spawn, spawn) {
  IoContext ctx;

  CoSpawn(ctx, Sum4());

  ctx.RunOnce();
}

Task<int> Delay(IoContext& ctx, int delay_s) {
  Timer timer(ctx);

  timer.ExpireAfter(delay_s);

  spdlog::info("Register Done");
  co_await timer.AsyncWait();

  spdlog::info("Delay task complete");
  co_return delay_s;
}

TEST(io_context, timer) {
  IoContext io_context;
  CoSpawn(io_context, Delay(io_context, 10));
  io_context.Run();
}
