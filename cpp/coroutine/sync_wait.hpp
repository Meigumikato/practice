#include <spdlog/spdlog.h>

#include <concepts>
#include <coroutine>
#include <exception>
#include <future>
#include <optional>
#include <type_traits>
#include <variant>

#include "manual_reset_event.hpp"

namespace detail {

template <typename Awaitable>
class SyncWaitTask;

template <typename ReturnType>
class SyncWaitTaskPromise {
 public:
  auto unhandled_exception() { result_ = std::current_exception(); }

  auto initial_suspend() -> std::suspend_always { return {}; }

  auto final_suspend() noexcept -> std::suspend_always {
    notify_.set_value(std::get<ReturnType>(result_));
    return {};
  }

  SyncWaitTask<ReturnType> get_return_object();

  template <typename Awaitable>
  Awaitable&& await_transform(Awaitable&& a) {
    return std::forward<Awaitable>(a);
  }

  std::suspend_never yield_value(ReturnType x) {
    result_ = x;
    return {};
  }

  ReturnType GetResult() {
    if (std::holds_alternative<ReturnType>(result_)) {
      return std::get<ReturnType>(result_);
    } else {
      std::rethrow_exception(std::get<std::exception_ptr>(result_));
    }
  }

  void SetNotify(std::promise<ReturnType>&& p) { notify_ = std::move(p); }

 private:
  std::promise<ReturnType> notify_;
  std::variant<ReturnType, std::exception_ptr> result_;
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

  void return_void() { notify_.set_value(); }

  void GetResult() {
    if (result_) {
      std::rethrow_exception(result_);
    }
  }

  void SetNotify(std::promise<void>&& p) { notify_ = std::move(p); }

 private:
  std::promise<void> notify_;
  std::exception_ptr result_;
};

template <typename ReturnType>
class SyncWaitTask {
 public:
  using promise_type = SyncWaitTaskPromise<ReturnType>;

  SyncWaitTask(std::coroutine_handle<promise_type> h) : h_(h) {}

  ~SyncWaitTask() {
    if (h_) {
      h_.destroy();
    }
  }

  void Start(std::promise<ReturnType> promise) {
    if (h_) {
      h_.promise().SetNotify(std::move(promise));
      h_.resume();
    }
  }

  void Wait() { h_.promise().event.Wait(); }

  auto Value() {
    if constexpr (std::is_same_v<ReturnType, void>) {
      h_.promise().GetResult();
      return;
    } else {
      return h_.promise().GetResult();
    }
  }

 private:
  std::coroutine_handle<promise_type> h_;
};

template <typename ReturnType>
inline SyncWaitTask<ReturnType> SyncWaitTaskPromise<ReturnType>::get_return_object() {
  return SyncWaitTask{std::coroutine_handle<SyncWaitTaskPromise>::from_promise(*this)};
}

inline SyncWaitTask<void> SyncWaitTaskPromise<void>::get_return_object() {
  return SyncWaitTask{std::coroutine_handle<SyncWaitTaskPromise>::from_promise(*this)};
}

template <typename Awaitable>
inline SyncWaitTask<typename Awaitable::ReturnType> MakeSyncWaitTask(Awaitable&& a) {
  if constexpr (std::is_same_v<typename Awaitable::ReturnType, void>) {
    co_await std::forward<Awaitable>(a);
  } else {
    co_yield co_await std::forward<Awaitable>(a);
  }
}

}  // namespace detail

template <typename Awaitable>
auto SyncWait(Awaitable&& a) {
  using ReturnType = typename Awaitable::ReturnType;

  auto task = detail::MakeSyncWaitTask(std::forward<Awaitable>(a));

  std::promise<ReturnType> promise;
  auto future = promise.get_future();
  task.Start(std::move(promise));

  if constexpr (std::is_same_v<void, ReturnType>) {
    future.get();
    return;
  } else {
    return future.get();
  }
}
