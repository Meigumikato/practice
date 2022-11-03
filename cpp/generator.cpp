#include <cassert>
#include <coroutine>
#include <iostream>
#include <iterator>
#include <concepts>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <coro/generator.hpp>

#include <spdlog/spdlog.h>
#include <utility>


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

  auto yield_value(std::remove_reference_t<T>& a) -> std::suspend_always {
    spdlog::info("GeneratorPromise lvalue reference yield_value {}", a);
    value = a;
    return {};
  }

  auto yield_value(std::remove_reference_t<T>&& a) -> std::suspend_always {
    spdlog::info("GeneratorPromise rvalue reference yield_value {}", a);
    value = a;
    return {};
  }

  T GetValue() {
    return value;
  }

  T value{0};
};


class GeneratorExhaustException : std::exception {
  public:

};


template<typename T>
class Generator {

 public:
  using promise_type = GeneratorPromise<T>;

  struct SentinelIterator {
  };

  struct GeneratorIterator {
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using reference = T&;
    using pointer = T*;

    T& operator * () {
      return handle_.promise().value;
    }

    T* operator -> () {
      return &handle_->promise().value();
    }

    // prefix
    GeneratorIterator& operator ++ () {

      handle_.resume();

      if (handle_.done()) {
        // throw std::logic_error("operator++");
      } 

      return *this;
    }

    // postfix
    GeneratorIterator operator ++ (int) {
      GeneratorIterator temp = *this;
      ++(*this);
      return temp;
    }

    friend bool operator== (const GeneratorIterator& a, const SentinelIterator&) {
      return a.handle_.done() || a.handle_ == nullptr;
    }

    friend bool operator== (const SentinelIterator&, const GeneratorIterator& a) {
      return a.handle_.done() || a.handle_ == nullptr;
    }

    friend bool operator!=(const GeneratorIterator& a, const SentinelIterator& b) {
      return !(a == b);
    }

    friend bool operator!=(const SentinelIterator& a, const GeneratorIterator& b) {
      return !(a == b);
    }

    std::coroutine_handle<promise_type> handle_;
  };

  Generator(std::coroutine_handle<promise_type> handle) : 
    handle_(handle) {
  }

  Generator(Generator&& other) : handle_(std::exchange(other.handle_, nullptr)) {
  }

  Generator& operator=(Generator&& other) {
    if (this == &other) {
      return *this;
    }

    if (handle_ != nullptr) {
      handle_.destroy();
    }

    handle_ = std::exchange(other.handle_, nullptr);
    return *this;
  }

  ~Generator() {
    if (handle_ != nullptr) {
      handle_.destroy();
    }
  }

  GeneratorIterator begin() {
    // because initial_suspend is suspend_always
    if (handle_ != nullptr) {
      handle_.resume();

      if (handle_.done()) {
        // throw std::logic_error("begin");
      }
    }

    return GeneratorIterator{handle_};
  }


  SentinelIterator end() {
    return SentinelIterator{};
  }


  template<typename... Args>
  static Generator<T> From(Args&&... args) {
    (co_yield std::forward<Args>(args), ...);
  }


  template<typename Func>
  Generator<std::invoke_result_t<Func, T>> Map(Func&& func) {
    for (auto ele : *this) {
      co_yield func(ele);
    }
  }


  template<typename Func>
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


template<typename T>
Generator<T> GeneratorPromise<T>::get_return_object() {
  return Generator<T>{
    std::coroutine_handle<GeneratorPromise<T>>::from_promise(*this)
  };
}



template<typename... Args>
Generator<int> Test(Args... args) {
  (co_yield std::forward<Args>(args), ...);
}


int main() {

  auto g1 = Test(1, 2, 3, 4);
  auto g2 = Test(5, 6, 7, 8);

  g2 = std::move(g1);

  g1 = std::move(g2);

  g2 = std::move(g1);

  auto filter_g = g2
    .Filter([](auto x) {
      return x % 2 == 0;
    })
  // ;
  // auto map_g = filter_g
    .Map([](auto x) -> char {
      return 'a' + x;
    });
  //   .Map([](auto x) -> char {
  //   return 'a' + x;
  // });

  // for (auto iter : g) {
  //   spdlog::info("generator {}", iter);
  // }
  //
  //
  // g.begin();

  // assert(g.begin() == g.end());

  // assert(g.begin() == g.end());

  // assert(map_g.begin() == map_g.end());
  int i = 0;
  for (auto x : filter_g) {
    ++i;
    spdlog::info("map {} {}", x, i);
  }

  return 0;
}

