#include <cassert>
#include <coroutine>
#include <iostream>
#include <iterator>
#include <concepts>
#include <type_traits>

#include <spdlog/spdlog.h>


template<typename T>
class Generator; 

template<typename T>
struct GeneratorPromise {

  void unhandled_exception() {}

  std::suspend_always initial_suspend() {
    return {};
  }

  std::suspend_always final_suspend() noexcept {
    return {};
  }

  Generator<T> get_return_object();

  void return_void() {}

  auto yield_value(T a) -> std::suspend_always {
    // spdlog::info("GeneratorPromise yield_value {}", a);
    value = a;
    return {};
  }

  T GetValue() {
    return value;
  }

  T value{0};
};



template<typename T>
class Generator {

 public:
  using promise_type = GeneratorPromise<T>;

  struct GeneratorIterator {
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using reference = T&;
    using pointer = T*;

    GeneratorIterator(std::coroutine_handle<promise_type> handle) : 
      handle_(handle) {
    }

    GeneratorIterator(const GeneratorIterator& other) : handle_(other.handle_) {
    }

    ~GeneratorIterator() {
    }

    T& operator * () {
      assert(handle_ != nullptr);
      return handle_.promise().value;
    }

    T* operator -> () {
      return &handle_->promise().value();
    }

    // prefix
    GeneratorIterator& operator ++ () {

      assert(handle_ != nullptr);
      handle_.resume();
      if (handle_.done()) {
        handle_ = nullptr;
        assert(handle_ == nullptr);
        return *this;
      } else {
        return *this;
      }
    }

    // postfix
    GeneratorIterator operator ++ (int) {
      GeneratorIterator temp = *this;
      ++(*this);
      return temp;
    }

    friend bool operator== (const GeneratorIterator& a, const GeneratorIterator& b) {
      return a.handle_ == b.handle_;
    }

    friend bool operator!=(const GeneratorIterator& a, const GeneratorIterator& b) {
      return !(a.handle_ == b.handle_);
    }

    std::coroutine_handle<promise_type> handle_;
  };

  Generator(std::coroutine_handle<promise_type> handle) : 
    handle_(handle) {

    // spdlog::info("Generator construct handle={}", fmt::ptr(&handle));
  }

  ~Generator() {
    handle_.destroy();
  }

  GeneratorIterator begin() {
    if (handle_.done()) {
      return end();
    } else {
      handle_.resume();
    }
    return GeneratorIterator{handle_};
  }


  GeneratorIterator end() {
    return GeneratorIterator{nullptr};
  }


  template<typename... Args>
  static Generator<T> From(Args&&... args) {
    (co_yield args, ...);
  }


  template<typename Func>
  Generator<std::invoke_result_t<Func, T>> Map(Func&& func) {
  }

 private:
  std::coroutine_handle<promise_type> handle_;
};


template<typename T>
Generator<T> GeneratorPromise<T>::get_return_object() {
  return Generator<T>{
    std::coroutine_handle<GeneratorPromise<T>>::from_promise(*this)
  };
}


Generator<int> Counter(int x = 10) {
  for (int i = 0; i < x; i++) {
    // spdlog::info("Counter suspend {}", i);
    co_yield i;
    // spdlog::info("Counter resume {}", i);
  }
}


int main() {

  auto x = Counter();

  for (auto iter : x) {
    spdlog::info("{}", iter);
  }

  return 0;
}

