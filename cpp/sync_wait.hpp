#include <coroutine>
#include <optional>
#include <mutex>
#include <condition_variable>
#include <type_traits>

#include <spdlog/spdlog.h>

template<typename Awaitable>
class SyncWaitTask;



template<typename Awaitable>
struct SyncWaitTaskPromise {

  using ResultType = typename Awaitable::ValueType;

  void unhandled_exception() {

  }

  auto initial_suspend() -> std::suspend_always {
    return {};
  }

  auto final_suspend() noexcept -> std::suspend_always {
    return {};
  }

  auto get_return_object() -> SyncWaitTask<Awaitable>;

  Awaitable await_transform(Awaitable&& a) {
    return std::forward<Awaitable>(a);
  }

  std::suspend_never yield_value(ResultType x) {

    spdlog::info("SyncWait yield_value");
    value_ = x;
    // cv_.notify_one();
    is_ok_ = true;

    return {};
  }

  bool is_ok_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::optional<ResultType> value_;
  std::optional<ResultType> result_;
};


template<typename AwaitAble>
class SyncWaitTask {
  public:
    using promise_type = SyncWaitTaskPromise<AwaitAble>;

    SyncWaitTask(std::coroutine_handle<promise_type> h) : h_(h) {}

    ~SyncWaitTask() {
      h_.destroy();
    }

    void Start() {
      h_.resume();
    }

    void Wait() {
      while(h_.promise().is_ok_) return;
    }

    std::optional<typename AwaitAble::ValueType> Value() {
      return h_.promise().value_;
    }

  private:
    std::coroutine_handle<promise_type> h_;
};

template<typename Awaitable>
auto SyncWaitTaskPromise<Awaitable>::get_return_object() -> SyncWaitTask<Awaitable> {
  return SyncWaitTask<Awaitable>{std::coroutine_handle<SyncWaitTaskPromise<Awaitable>>::from_promise(*this)};
}

template<typename Awaitable>
SyncWaitTask<Awaitable> MakeSyncWaitTask(Awaitable&& a) {
  co_yield co_await std::forward<Awaitable>(a);
}

template<typename Awaitable>
auto SyncWait(Awaitable&& a) { 

  auto task = MakeSyncWaitTask(std::forward<Awaitable>(a));
  task.Start();

  task.Wait();

  spdlog::info("");

  return task.Value();
}



