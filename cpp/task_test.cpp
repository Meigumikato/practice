#include "task.hpp"
#include "sync_wait.hpp"


Task<int> Take() {
  co_return 42;
}

Task<int> Sum(int a, int c) {
  co_return a + c;
}

Task<int> Take42() {
  auto x2 = co_await Take();
  auto x1 = co_await Take();
  co_return co_await Sum(x2, x1);
}
 
int main() {
  SyncWait(Take42());

  return 0;
}
