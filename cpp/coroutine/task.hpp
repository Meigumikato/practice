#include <spdlog/spdlog.h>

#include <coroutine>
#include <exception>
#include <variant>

template <typename T = void>
class TaskPromise;

template <typename T = void>
class Task;

template <typename T>
class TaskPromiseBase {
 public:
  struct FinalAwaiter {
    auto await_ready() noexcept -> bool { return false; }

    template <typename Promise>
    auto await_suspend(std::coroutine_handle<Promise> h) noexcept
        -> std::coroutine_handle<> {
      auto& promise = h.promise();
      if (promise.continuation_ && !promise.continuation_.done()) {
        return promise.continuation_;
      } else {
      }

      return std::noop_coroutine();
    }

    auto await_resume() noexcept {}
  };

  Task<T> get_return_object() noexcept;

  auto initial_suspend() -> std::suspend_always { return {}; }

  FinalAwaiter final_suspend() noexcept { return {}; }

  void SetContinuation(std::coroutine_handle<> h) { continuation_ = h; }

 private:
  std::coroutine_handle<> continuation_{nullptr};
};

template <typename T>
class TaskPromise : public TaskPromiseBase<T> {
 public:
  using CoroutineHandleType = std::coroutine_handle<TaskPromise>;

  auto get_return_object() noexcept -> Task<T>;

  auto unhandled_exception() noexcept { result_ = std::current_exception(); }

  auto return_value(T result) noexcept { result_ = result; }

  auto GetResult() -> T {
    if (std::holds_alternative<T>(result_)) {
      return std::get<T>(result_);
    } else {
      std::rethrow_exception(std::get<std::exception_ptr>(result_));
    }
  }

 private:
  std::variant<T, std::exception_ptr> result_;
};

template <>
class TaskPromise<void> : public TaskPromiseBase<void> {
 public:
  using CoroutineHandleType = std::coroutine_handle<TaskPromise>;

  auto get_return_object() noexcept -> Task<>;

  auto unhandled_exception() noexcept { result_ = std::current_exception(); }

  auto return_void() {
    if (result_) {
      std::rethrow_exception(result_);
    }
  }

 private:
  std::exception_ptr result_;
};

template <typename T>
class [[nodiscard]] Task {
 public:
  using ReturnType = T;
  using promise_type = TaskPromise<T>;
  using CoroutineHandleType = typename TaskPromise<T>::CoroutineHandleType;

  class Awaiter {
   public:
    explicit Awaiter(CoroutineHandleType h) noexcept : coro_(h) {}

    auto await_ready() noexcept -> bool { return !coro_ || coro_.done(); }

    CoroutineHandleType await_suspend(std::coroutine_handle<> parent) noexcept {
      coro_.promise().SetContinuation(parent);
      return coro_;
    }

    auto await_resume() {
      if constexpr (!std::is_same_v<T, void>) {
        return coro_.promise().GetResult();
      }
    }

   private:
    CoroutineHandleType coro_;
  };

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

    if (coro_) {
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

  CoroutineHandleType Detach() {
    CoroutineHandleType temp = coro_;
    coro_ = nullptr;
    return temp;
  }

  // auto Resume() { coro_.resume(); }

  auto operator co_await() && noexcept -> Awaiter { return Awaiter{coro_}; }

  // Awaiter operator co_await() & noexcept {
  //   return Awaiter{coro_};
  // }

 private:
  friend class TaskPromise<T>;

  explicit Task(CoroutineHandleType h) noexcept : coro_(h) {}

  CoroutineHandleType coro_;
};

template <typename T>
inline auto TaskPromise<T>::get_return_object() noexcept -> Task<T> {
  return Task<T>{TaskPromise::CoroutineHandleType::from_promise(*this)};
}

inline auto TaskPromise<>::get_return_object() noexcept -> Task<> {
  return Task<>{TaskPromise::CoroutineHandleType::from_promise(*this)};
}
