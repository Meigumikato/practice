#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <cassert>
#include <concepts>
#include <coroutine>
#include <exception>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

template <typename T>
class Generator;

template <typename T>
class GeneratorPromise {
 public:

  void unhandled_exception() { result_ = std::current_exception(); }

  auto initial_suspend() -> std::suspend_always { return {}; }

  auto final_suspend() noexcept -> std::suspend_always { return {}; }

  Generator<T> get_return_object();

  void return_void() {}

  auto yield_value(std::remove_reference_t<T>& a) -> std::suspend_always {
    result_ = std::addressof(a);
    return {};
  }

  auto yield_value(std::remove_reference_t<T>&& a) -> std::suspend_always {
    result_ = std::addressof(a);
    return {};
  }

  auto GetValue() {
    if (std::holds_alternative<std::exception_ptr>(result_)) {
      std::rethrow_exception(std::get<std::exception_ptr>(result_));
    } else {
      return std::get<T*>(result_);
    }
  }

 private:
  std::variant<T*, std::exception_ptr> result_;
};

class GeneratorExhaustException : std::exception {
 public:
};

template <typename T>
class Generator {
 public:
  using promise_type = GeneratorPromise<T>;

  struct SentinelIterator {};

  struct GeneratorIterator {
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using reference = T&;
    using pointer = T*;

    reference operator*() { return *handle_.promise().GetValue(); }

    pointer operator->() { return handle_->promise().GetValue(); }

    // prefix
    GeneratorIterator& operator++() {
      if (!handle_ || handle_.done()) {
        throw std::logic_error("Generator Coroutine Exhaust");
      }

      handle_.resume();

      return *this;
    }

    // postfix
    GeneratorIterator operator++(int) {
      GeneratorIterator temp = *this;
      ++(*this);
      return temp;
    }

    friend bool operator==(const GeneratorIterator& a, const SentinelIterator&) {
      return a.handle_.done() || !a.handle_;
    }

    friend bool operator==(const SentinelIterator&, const GeneratorIterator& a) {
      return a.handle_.done() || !a.handle_;
    }

    friend bool operator!=(const GeneratorIterator& a, const SentinelIterator& b) { return !(a == b); }

    friend bool operator!=(const SentinelIterator& a, const GeneratorIterator& b) { return !(a == b); }

    std::coroutine_handle<promise_type> handle_;
  };

  Generator(std::coroutine_handle<promise_type> handle) : handle_(handle) {}

  Generator(Generator&& other) : handle_(std::exchange(other.handle_, nullptr)) {}

  Generator& operator=(Generator&& other) {
    if (this == &other) {
      return *this;
    }

    if (handle_) {
      handle_.destroy();
    }

    handle_ = std::exchange(other.handle_, nullptr);
    return *this;
  }

  ~Generator() {
    if (handle_) {
      handle_.destroy();
    }
  }

  GeneratorIterator begin() {
    // because initial_suspend is suspend_always
    if (!handle_ && handle_.done()) {
      spdlog::throw_spdlog_ex("generator is exhaust");
    }

    handle_.resume();
    return GeneratorIterator{handle_};
  }

  SentinelIterator end() { return SentinelIterator{}; }

  template <typename... Args>
  static Generator<T> From(Args&&... args) {
    (co_yield std::forward<Args>(args), ...);
  }

  template <typename Func>
  Generator<std::invoke_result_t<Func, T>> Map(Func&& func) {
    for (auto ele : *this) {
      co_yield func(ele);
    }
  }

  template <typename Func>
  Generator<T> Filter(Func&& func) {
    for (auto ele : *this) {
      if (func(ele)) {
        co_yield ele;
      }
    }
  }

 private:
  std::coroutine_handle<promise_type> handle_;
};

template <typename T>
Generator<T> GeneratorPromise<T>::get_return_object() {
  return Generator<T>{std::coroutine_handle<GeneratorPromise<T>>::from_promise(*this)};
}
