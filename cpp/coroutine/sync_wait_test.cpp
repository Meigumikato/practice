#include <thread>

#include <gtest/gtest.h>

#include "task.hpp"
#include "sync_wait.hpp"

Task<int> Just(int i) { co_return i; }

Task<int> Sum(int a, int b) { 
  auto a1 = co_await Just(a);
  auto b1 = co_await Just(b);
  co_return a1 + b1; 
}

Task<int> Sum4() {
  auto x2 = co_await Sum(1, 2);
  auto x1 = co_await Sum(2, 3);
  co_return co_await Sum(x2, x1);
}

TEST(sync_wait, base) { SyncWait(Sum4()); }
