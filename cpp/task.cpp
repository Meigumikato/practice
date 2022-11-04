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
  // TaskPromise() noexcept = default;
  // ~TaskPromise() = default;

  friend struct FinalAwaiter;

  struct FinalAwaiter {
    bool await_ready() noexcept {
      spdlog::info("final_suspend await_suspend not suspend");
      return false;
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<TaskPromise> h) noexcept {

      auto& promise = h.promise();
      // resume parent
      if (promise.continuation_ != nullptr) {
        spdlog::info("final_suspend await_suspend resume parent={}", fmt::ptr(&promise.continuation_));
        return promise.continuation_;
      } 

      return std::noop_coroutine();

      spdlog::info("final_suspend await_suspend");
    }

    void await_resume() noexcept {
      spdlog::info("final_suspend await_resume");
    }
  };

  Task<T> get_return_object() noexcept;

  std::suspend_always initial_suspend() noexcept {
    spdlog::info("initial_suspend suspend_always");
    return {};
  }

  FinalAwaiter final_suspend() noexcept {
    return {};
  }

  void unhandled_exception() noexcept {
    result_ = std::current_exception();
  }

  void return_value(T result) noexcept {
    spdlog::info("TaskPromise return_value return_value={}", result);
    result_ = result;
  }

private:
  friend struct Awaiter<T>;
  friend class Task<T>;

  std::coroutine_handle<> continuation_;
  std::variant<std::monostate, T, std::exception_ptr> result_;
};

template<typename T>
struct Awaiter {

  using promise_type = typename Task<T>::promise_type;

  explicit Awaiter(std::coroutine_handle<promise_type> h) noexcept :
    coro_(h) {}

  bool await_ready() noexcept {

    spdlog::info("Awaiter await_ready {}", !coro_ ||coro_.done());
    return !coro_ || coro_.done();
  }

  std::coroutine_handle<promise_type> await_suspend(std::coroutine_handle<promise_type> parent) noexcept {
    coro_.promise().continuation_ = parent;
    spdlog::info("Awaiter await_suspend continuation={} current={}", fmt::ptr(&coro_.promise().continuation_), fmt::ptr(&coro_));
    return coro_;
  }

  T await_resume() {

    T x = std::get<T>(coro_.promise().result_);
    spdlog::info("Awaiter await_resume return_value={}", x);

    return x;
  }

 private:
  friend class Task<T>;
  std::coroutine_handle<promise_type> coro_;
};

template<typename T>
class Task {
public:

  using promise_type = TaskPromise<T>;

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

  Task& operator=(Task&& t) noexcept;


  void Resume() {
    coro_.resume();
  }

  T Value() {
    return std::get<int>(coro_.promise().result_);
  }

  Awaiter<T> operator co_await() && noexcept {
    return Awaiter<T>{coro_};
  }

 private:
  friend class TaskPromise<T>;

  explicit Task(std::coroutine_handle<promise_type> h) noexcept : coro_(h) {

  }
  std::coroutine_handle<promise_type> coro_;
};


template<typename T>
Task<T> TaskPromise<T>::get_return_object() noexcept {
  return Task<T>{std::coroutine_handle<typename Task<T>::promise_type>::from_promise(*this)};
}


Task<int> Take() {
  co_return 42;
}

Task<int> Sum(int a, int c) {
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

  int a = 0;
  std::cin >> a;
  x.Resume();

  std::cout << x.Value();

  return 0;
}




