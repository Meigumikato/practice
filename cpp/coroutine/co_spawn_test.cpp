#include "co_spawn.hpp"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <unistd.h>

#include <thread>

#include "io_context.hpp"
#include "static_thread_pool.hpp"
#include "sync_wait.hpp"
#include "task.hpp"

int x = 0;

Task<int> Take() { co_return 42; }

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
//
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
  CoSpawn(io_context, Delay(io_context, 1));
  io_context.RunOnce();
  io_context.RunOnce();
}

Task<int> Transfer(StaticThreadPool& pool) {
  std::cout << " before" << std::this_thread::get_id() << std::endl;
  co_await pool.Schedule();
  std::cout << " after" << std::this_thread::get_id() << std::endl;
  // sleep(1);
  co_return 10;
}

TEST(static_thread_pool, transfer) {
  StaticThreadPool pool(1);
  //
  auto task = Transfer(pool);

  ASSERT_EQ(SyncWait(std::move(task)), 10);
  ASSERT_EQ(SyncWait(Transfer(pool)), 10);
}
