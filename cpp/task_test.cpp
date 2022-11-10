#include "task.hpp"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include "sync_wait.hpp"

Task<> Nothing() { co_return; }

Task<int> Take() {
  co_return 42;
}

Task<int> Sum(int a, int c) { co_return a + c; }

Task<int> Sum4() {
  auto x2 = co_await Take();
  auto x1 = co_await Take();
  co_return co_await Sum(x2, x1);
}

TEST(task, co_return) {
  ASSERT_NO_FATAL_FAILURE(SyncWait(Nothing()));
  ASSERT_EQ(SyncWait(Take()), 42);
  ASSERT_EQ(84, SyncWait(Sum(42, 42)));
}


TEST(task, co_await) {
  ASSERT_EQ(SyncWait(Sum4()), 84);

  auto task = [] (int a, int b, int c, int d) -> Task<int> {
    int temp1 = co_await Sum(a, b);
    int temp2 = co_await Sum(c, d);
    co_return co_await Sum(temp1, temp2);
  };

  ASSERT_EQ(1 + 2 + 3 + 4, SyncWait(task(1, 2, 3, 4)));
}


struct Awaiter {
  auto await_ready() { return false; }
  
  auto await_suspend(std::coroutine_handle<> h) { return h; }

  auto await_resume() {}
};

Task<> test1() {
  co_await Awaiter{};
}

Task<> test2() {
  co_await test1();
  co_await Awaiter{};
}


TEST(task, awaiter) {
  SyncWait(test1());
  SyncWait(test2());
}
