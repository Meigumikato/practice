#include "generator.hpp"

#include <gtest/gtest.h>
#include <string>

struct Element {
  std::string name;
  int number;
};

Generator<int> Counter() {
  for (int i = 0;; ++i) {
    co_yield i;
  }
}

Generator<Element> GenElement() {
  Element ele;
  for (int i = 0;; ++i) {
    co_yield { .name = std::to_string(i), .number = i };
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

TEST(generator, access) {

  for (const auto& ele : GenElement()) {

    if (ele.number > 10) break;
    ASSERT_EQ(std::to_string(ele.number), ele.name);
    ASSERT_EQ(ele.number, std::stoi(ele.name));
  }
}
