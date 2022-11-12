#include <algorithm>
#include <cmath>
#include <cstdio>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/take.hpp>

#include <iterator>



#include <set>
#include <range/v3/all.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <ostream>
#include <iostream>
#include <source_location>

#include <memory>


using namespace ranges;

struct Test {

  Test() {
    std::source_location sl = std::source_location::current();
    std::cout << sl.function_name() << "\n";
  }

  Test(Test&& t) {
    std::source_location sl = std::source_location::current();
    a = t.a;
    t.a = 0;
    std::cout << sl.function_name() << "\n";
  }

  void Moveable(std::source_location sc) && {
    std::cout << "Moveable" << sc.function_name() << (int)sc.line() << (int)sc.column() << "\n";
  }


  ~Test() {
    std::source_location sl = std::source_location::current();
    std::cout << sl.function_name() << "\n";
  }
  int a{};
};


void Move(Test&& t) {
  std::cout << "Move\n";
  ++t.a;
}

int main() {
  Test t;

  Test t1 = std::move(t);


  std::move(t1).Moveable(std::source_location::current());
  // Move(std::move(t));
  // std::cout << t1.a << std::endl;
  // std::cout << t.a << std::endl;

  return 0;
}
