
#include "task.hpp"

#include <gtest/gtest.h>

inline Task<int> Just(int i) { co_return i; }

inline Task<int> Sum(int a, int b) { 
  auto a1 = co_await Just(a);
  auto b1 = co_await Just(b);
  co_return a1 + b1; 
}

inline Task<int> Sum4() {
  auto x2 = co_await Sum(1, 2);
  auto x1 = co_await Sum(2, 3);
  co_return co_await Sum(x2, x1);
}
