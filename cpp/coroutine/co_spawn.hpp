#include <coroutine>
#include <utility>


template <typename Executor, typename Awaitable>
constexpr inline void CoSpawn(Executor& ex, Awaitable&& awaitable) {
  ex.Execute(std::forward<Awaitable>(awaitable));
}


