#include <spdlog/spdlog.h>

#include <concepts>
#include <coroutine>
#include <exception>
#include <optional>
#include <type_traits>
#include <variant>

#include "manual_reset_event.hpp"

template <typename Awaitable>
class SyncWaitTask;

template <typename ResultType>
class SyncWaitTaskPromise {
 public:
  auto unhandled_exception() { result_ = std::current_exception(); }

  auto initial_suspend() -> std::suspend_always { return {}; }

  auto final_suspend() noexcept -> std::suspend_always { return {}; }

  SyncWaitTask<ResultType> get_return_object();

  template <typename Awaitable>
  Awaitable await_transform(Awaitable&& a) {
    return std::forward<Awaitable>(a);
  }

  std::suspend_never yield_value(ResultType x) {
    result_ = x;
    return {};
  }

  ResultType GetResult() {
    if (std::holds_alternative<ResultType>(result_)) {
      return std::get<ResultType>(result_);
    } else {
      std::rethrow_exception(std::get<std::exception_ptr>(result_));
    }
  }

 private:
  std::variant<ResultType, std::exception_ptr> result_;
};

template <>
class SyncWaitTaskPromise<void> {
 public:
  auto unhandled_exception() { result_ = std::current_exception(); }

  auto initial_suspend() -> std::suspend_always { return {}; }

  auto final_suspend() noexcept -> std::suspend_always { return {}; }

  SyncWaitTask<void> get_return_object();

  template <typename Awaitable>
  Awaitable await_transform(Awaitable&& a) {
    return std::forward<Awaitable>(a);
  }

  void return_void() {}

  void GetResult() {
    if (result_) {
      std::rethrow_exception(result_);
    }
  }

 private:
  std::exception_ptr result_;
};

template <typename ResultType>
class SyncWaitTask {
 public:
  using promise_type = SyncWaitTaskPromise<ResultType>;

  SyncWaitTask(std::coroutine_handle<promise_type> h) : h_(h) {}

  ~SyncWaitTask() {
    if (h_) {
      h_.destroy();
    }
  }

  void Start(ManualResetEvent& event) {
    if (h_) {
      event.Set();
      h_.resume();
    }
  }

  void Wait() { h_.promise().event.Wait(); }

  auto Value() {
    if constexpr (std::is_same_v<ResultType, void>) {
      h_.promise().GetResult();
      return;
    } else {
      return h_.promise().GetResult();
    }
  }

 private:
  std::coroutine_handle<promise_type> h_;
};

template <typename ResultType>
inline SyncWaitTask<ResultType> SyncWaitTaskPromise<ResultType>::get_return_object() {
  return SyncWaitTask{std::coroutine_handle<SyncWaitTaskPromise>::from_promise(*this)};
}

inline SyncWaitTask<void> SyncWaitTaskPromise<void>::get_return_object() {
  return SyncWaitTask{std::coroutine_handle<SyncWaitTaskPromise>::from_promise(*this)};
}

template <typename Awaitable>
requires(!std::same_as<void, typename Awaitable::ResultType>) inline SyncWaitTask<
    typename Awaitable::ResultType> MakeSyncWaitTask(Awaitable&& a) {
  co_yield co_await std::forward<Awaitable>(a);
}

template <typename Awaitable>
requires(std::same_as<void, typename Awaitable::ResultType>) inline SyncWaitTask<
    typename Awaitable::ResultType> MakeSyncWaitTask(Awaitable&& a) {
  co_await std::forward<Awaitable>(a);
}

template <typename Awaitable>
auto SyncWait(Awaitable&& a) {
  auto task = MakeSyncWaitTask(std::forward<Awaitable>(a));
  ManualResetEvent event;
  task.Start(event);

  event.Wait();

  return task.Value();
}
