#include <coroutine>
#include <exception>
#include <variant>


class task;
struct awaiter;

class promise_type {
 public:
  promise_type() noexcept = default;
  ~promise_type() = default;

  struct final_awaiter {
    bool await_ready() noexcept;
    std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type> h) noexcept;
    void await_resume() noexcept;
  };

  task get_return_object() noexcept;

  std::suspend_always initial_suspend() noexcept {
    return {};
  }

  final_awaiter final_suspend() noexcept {
    return {};
  }

  void unhandled_exception() noexcept {
    result_ = std::current_exception();
  }

  void return_value(int result) noexcept {
    result_ = result;
  }

private:
  // friend task::awaiter;
  std::coroutine_handle<> continuation_;
  std::variant<std::monostate, int, std::exception_ptr> result_;
};

class task {
public:

    task(task&& t) noexcept;
    ~task() {

    }

    task& operator=(task&& t) noexcept;

    struct awaiter {
      explicit awaiter(std::coroutine_handle<promise_type> h) noexcept;
      bool await_ready() noexcept;
      std::coroutine_handle<promise_type> await_suspend(
          std::coroutine_handle<> h) noexcept;
      int await_resume();
     private:
      std::coroutine_handle<promise_type> coro_;
    };

    awaiter operator co_await() && noexcept;

private:
    explicit task(std::coroutine_handle<promise_type> h) noexcept;

    std::coroutine_handle<promise_type> coro_;
};
