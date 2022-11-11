#include <coroutine>
#include <iostream>
#include <string>
#include <utility>

using namespace std::string_literals;

struct Awaiter {};

// promise_type wrapper type
struct Chat {
  struct promise_type {
    // store a value from or for the coroutine
    std::string msg_out{}, msg_in{};

    // what to do in case of an exception
    void unhandled_exception() noexcept {}

    // coroutine creation
    Chat get_return_object() { return Chat{this}; }

    // startup suspend custom point
    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }

    std::suspend_always yield_value(std::string msg) noexcept {
      std::cout << "Trace:yield_value\n";
      msg_out = std::move(msg);
      return {};
    }

    auto await_transform(std::string) noexcept {
      struct Awaiter {
        promise_type& pt;
        constexpr bool await_ready() const noexcept { return true; }
        std::string await_resume() const noexcept { return std::move(pt.msg_in); }
        void await_suspend(std::coroutine_handle<>) const noexcept {}
      };

      std::cout << "Trace:await_transform\n";

      return Awaiter{*this};
    }

    // value from co_return
    void return_value(std::string msg) noexcept { msg_out = std::move(msg); }
  };

  using Handle = std::coroutine_handle<promise_type>;

  Handle co_handle;

  // get handle from promise_type
  explicit Chat(promise_type* p) : co_handle({Handle::from_promise(*p)}) {}

  // move only
  Chat(Chat&& rhs) : co_handle{std::exchange(rhs.co_handle, nullptr)} {}

  ~Chat() {
    if (co_handle) {
      co_handle.destroy();
    }
  }

  std::string listen() {
    if (!co_handle.done()) {
      co_handle.resume();
    }
    return std::move(co_handle.promise().msg_out);
  }

  // send data to the coroutine and activate it.
  void answer(std::string msg) {
    co_handle.promise().msg_in = msg;
    if (!co_handle.done()) {
      co_handle.resume();
    }
  }
};

Chat Fun() {
  // call promise_type.yield_value
  co_yield "Hello!\n"s;

  // call promise_type.await_transform
  std::cout << co_await std::string{};

  // call promise_type.return_value
  co_return "Here!\n"s;
}

void Use() {
  Chat chat = Fun();

  std::cout << chat.listen();

  chat.answer("Where are you?\n");

  std::cout << chat.listen();
}

int main() {
  Use();
  return 0;
}
