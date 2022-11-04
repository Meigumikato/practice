#include "spdlog/fmt/bundled/format.h"
#include <coroutine>
#include <exception>
#include <iostream>
#include <utility>
#include <variant>
#include <spdlog/spdlog.h>

#include <coro/task.hpp>



template<typename T>
class Task;

template<typename T>
struct Awaiter;

template<typename T>
class TaskPromise {
 public:

  using co_handle = std::coroutine_handle<TaskPromise>;

  struct SuspendAlways {

    SuspendAlways(TaskPromise& promise) : promise(promise) {
    }

    bool await_ready() const noexcept { 
      spdlog::info("{} on initial_suspend stage suspend ready is {}",
                   fmt::ptr(co_handle::from_promise(promise).address()),
                   !false);
      return false; 
    }

    void await_suspend(std::coroutine_handle<> h) const noexcept {
      spdlog::info("{} on initial_suspend stage suspended", fmt::ptr(h.address()));
    }

    void await_resume() const noexcept {
      spdlog::info("{} on initial_suspend stage resume to execute logic",
                   fmt::ptr(co_handle::from_promise(promise).address()));
    }

    TaskPromise& promise;
  };

  struct FinalAwaiter {
    FinalAwaiter(TaskPromise& promise) : promise(promise) {
    }

    bool await_ready() noexcept {
      spdlog::info("{} on final_suspend stage is ready to suspend",
                   fmt::ptr(co_handle::from_promise(promise).address()));
      return false;
    }

    std::coroutine_handle<> await_suspend(co_handle h) noexcept {
      auto& promise = h.promise();
      // resume parent
      if (promise.continuation_ != nullptr) {
        spdlog::info("{} on final_suspend stage is suspend, ready to resume parent={}", 
                     fmt::ptr(co_handle::from_promise(promise).address()),
                     fmt::ptr(&promise.continuation_));
        return promise.continuation_;
      } 

      return std::noop_coroutine();
    }

    void await_resume() noexcept {
      spdlog::info("{} on final_suspend stage is resume",
                   fmt::ptr(co_handle::from_promise(promise).address()));
    }

    TaskPromise& promise;
  };

  Task<T> get_return_object() noexcept;

  SuspendAlways initial_suspend() noexcept {
    spdlog::info("{} on co_await initial_suspend stage", 
                 fmt::ptr(co_handle::from_promise(*this).address()));
    return {*this};
  }

  FinalAwaiter final_suspend() noexcept {
    spdlog::info("{} on co_await final_suspend stage",
                 fmt::ptr(co_handle::from_promise(*this).address()));
    return {*this};
  }

  void unhandled_exception() noexcept {
    result_ = std::current_exception();
  }

  void return_value(T result) noexcept {
    spdlog::info("{} on co_return stage  return value={}",
                 fmt::ptr(co_handle::from_promise(*this).address()),
                 result);
    result_ = result;
  }

  T Value() {

    if (std::holds_alternative<T>(result_)) {
      return std::get<int>(result_);
    } else {
      std::rethrow_exception(std::get<std::exception_ptr>(result_));
    }
  }

  void SetContinuation(co_handle h) {
    continuation_ = h;
  }

 private:
  friend struct Awaiter<T>;

  co_handle continuation_;
  std::variant<std::monostate, T, std::exception_ptr> result_;
};

template<typename T>
struct Awaiter {
  // using promise_type = TaskPromise<T>;
  using co_handle = typename TaskPromise<T>::co_handle;

  explicit Awaiter(co_handle h) noexcept :
    coro_(h) {}

  bool await_ready() noexcept {
    spdlog::info("{} on customalization co_await stage suspend ready status is {}", 
                 fmt::ptr(coro_.address()), 
                 !coro_ ||coro_.done());

    return !coro_ || coro_.done();
  }

  co_handle await_suspend(co_handle parent) noexcept {
    coro_.promise().SetContinuation(parent);
    spdlog::info("{} on customalization co_await stage is suspended and continuation is {}", 
                 fmt::ptr(coro_.promise().continuation_.address()),
                 fmt::ptr(coro_.address()));
    return coro_;
  }

  T await_resume() {
    T x = coro_.promise().Value();
    spdlog::info("{} on await_resume resume co_await return is {}\n",
                 fmt::ptr(coro_.address()), x);
    return x;
  }

 private:
  friend class Task<T>;
  co_handle coro_;
};

template<typename T>
class Task {
 public:

  using promise_type = TaskPromise<T>;
  using co_handle = typename TaskPromise<T>::co_handle;

  Task(Task&& t) noexcept {
    if (&t == this) {
      return;
    }

    coro_.destroy();
    coro_ = std::exchange(t.coro_, nullptr);
  }

  ~Task() {
    coro_.destroy();
  }

  Task& operator=(Task&& t) noexcept {
    if (&t == this) {
      return *this;
    }

    t.coro_.destroy();
    coro_ = std::exchange(t.coro_, nullptr);
    return *this;
  }


  void Resume() {
    coro_.resume();
  }

  void SpawnOn() {
  }

  T Value() {
    return coro_.promise().Value();
  }

  Awaiter<T> operator co_await() && noexcept {
    return Awaiter<T>{coro_};
  }

 private:
  friend class TaskPromise<T>;

  explicit Task(co_handle h) noexcept : coro_(h) {}

  co_handle coro_;
};


template<typename T>
Task<T> TaskPromise<T>::get_return_object() noexcept {
  return Task<T>{TaskPromise::co_handle::from_promise(*this)};
}



Task<int> Take() {
  co_return 42;
}

Task<int> Sum(int a, int c) {
  spdlog::error("sleep");
  sleep(1);
  co_return a + c;
}

Task<int> Take42() {
  auto x2 = co_await Take();
  auto x1 = co_await Take();
  co_return co_await Sum(x2, x1);
}
 
int main() {
  auto x = Take42();

  std::cout << x.Value();

  return 0;
}
