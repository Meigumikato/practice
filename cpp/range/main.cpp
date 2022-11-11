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


using namespace ranges;


int LagestFactor(int x) {

  int base = 2;

  while (x % base != 0) {
    if (base >= x / 2) break;
    ++base;
  }

  if (x % base != 0) {
    return 1;
  } else {
    return x / base;
  }

  return x;
}


int main() {

  std::ostringstream oss;
  std::ostream_iterator<int> s(oss);

  std::set<int> a {1, 2, 4, 5};

  oss.clear();

  std::copy(a.begin(), a.end(), s);

  std::cout << oss.str();

  return 0;
}
