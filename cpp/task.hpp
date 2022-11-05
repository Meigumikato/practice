#include <coroutine>
#include <exception>
#include <variant>

#include <spdlog/spdlog.h>


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
        spdlog::info("{} on final_suspend stage is suspended, ready to resume parent={}", 
                     fmt::ptr(co_handle::from_promise(promise).address()),
                     fmt::ptr(promise.continuation_.address()));
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

  void SetContinuation(std::coroutine_handle<> h) {
    continuation_ = h;
  }

 private:
  friend struct Awaiter<T>;

  std::coroutine_handle<> continuation_;
  std::variant<std::monostate, T, std::exception_ptr> result_;
};

template<typename T>
struct Awaiter {
  // using promise_type = TaskPromise<T>;
  using co_handle = typename TaskPromise<T>::co_handle;

  explicit Awaiter(co_handle h) noexcept :
    coro_(h) {}

  bool await_ready() noexcept {
    spdlog::info("co_await caller on {} customalization co_await stage suspend ready status is {}", 
                 fmt::ptr(coro_.address()), 
                 !coro_ ||coro_.done());

    return !coro_ || coro_.done();
  }

  co_handle await_suspend(std::coroutine_handle<> parent) noexcept {
    coro_.promise().SetContinuation(parent);
    spdlog::info("{} on customalization co_await stage is suspended and {} is resumed", 
                 fmt::ptr(coro_.promise().continuation_.address()),
                 fmt::ptr(coro_.address()));
    return coro_;
  }

  T await_resume() {
    T x = coro_.promise().Value();
    spdlog::info("{} on customalization co_await stage is resumed, co_return value is {}\n",
                 fmt::ptr(coro_.promise().continuation_.address()), x);
    return x;
  }

 private:
  friend class Task<T>;
  co_handle coro_;
};

template<typename T>
class Task {
 public:

  using ValueType = T;
  using promise_type = TaskPromise<T>;
  using co_handle = typename TaskPromise<T>::co_handle;

  Task(Task&& t) noexcept {
    if (&t == this) {
      return;
    }

    coro_ = std::exchange(t.coro_, nullptr);
  }

  Task& operator=(Task&& t) noexcept {
    if (&t == this) {
      return *this;
    }

    if (coro_ != nullptr) {
      t.coro_.destroy();
    }

    coro_ = std::exchange(t.coro_, nullptr);
    return *this;
  }

  ~Task() {
    if (coro_) {
      coro_.destroy();
    }
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

  Awaiter<T> operator co_await() & noexcept {
    return Awaiter<T>{coro_};
  }

 private:
  friend class TaskPromise<T>;

  explicit Task(co_handle h) noexcept : coro_(h) {}

  co_handle coro_{nullptr};
};


template<typename T>
Task<T> TaskPromise<T>::get_return_object() noexcept {
  spdlog::info("{} get_return_object", fmt::ptr(TaskPromise::co_handle::from_promise(*this).address()));
  return Task<T>{TaskPromise::co_handle::from_promise(*this)};
}
