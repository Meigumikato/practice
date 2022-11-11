#include "generator.hpp"

#include <gtest/gtest.h>
#include <string>

#include <folly/experimental/coro/Generator.h>
#include <spdlog/spdlog.h>

struct Element {
  std::string name;
  int number;

  // Element(int i) {
  //   number = i;
  //   name = std::to_string(i);
  // }
  //
  // ~Element() {
  //   spdlog::info("Element destory");
  // }
};

struct Element1 {
  std::string name;
  int number;

  Element1(int i) {
    number = i;
    name = std::to_string(i);
  }

  ~Element1() {
    spdlog::info("Element destory");
  }
};

Generator<int> Counter() {
  for (int i = 0;; ++i) {
    co_yield i;
  }
}

folly::coro::Generator<Element> GenElement() {
  for (int i = 0;; ++i) {
    std::string x = std::to_string(i);
    co_yield Element{x, i};
  }
}

folly::coro::Generator<Element1> GenElement1() {
  for (int i = 0;; ++i) {
    co_yield Element1{i};
  }
}


TEST(generator, counter) {
  ASSERT_NO_FATAL_FAILURE(
    for (auto i : Counter()) {
      if (i > 10) break;
    }
  );

  ASSERT_NO_FATAL_FAILURE(
    for (auto& i : Counter()) {
      if (i > 10) break;
    }
  );

  auto gen = Counter();
  for (auto iter = gen.begin(); iter != gen.end(); ++iter) {

    ASSERT_NO_FATAL_FAILURE(*iter);
    if (auto val = *iter; val > 10) break;
  }
}

TEST(generator, access1) {

  for (const auto& ele : GenElement()) {

    if (ele.number > 10) break;
    ASSERT_EQ(std::to_string(ele.number), ele.name);
    ASSERT_EQ(ele.number, std::stoi(ele.name));
  }
}

TEST(generator, access) {

  for (const auto& ele : GenElement1()) {

    if (ele.number > 10) break;
    ASSERT_EQ(std::to_string(ele.number), ele.name);
    ASSERT_EQ(ele.number, std::stoi(ele.name));
  }
}
