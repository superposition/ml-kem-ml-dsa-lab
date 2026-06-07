#include <cassert>

#include "pqcore/field.hpp"

int main() {
  using pqcore::ModQ;

  assert(ModQ<3329>{3330}.value() == 1);
  assert(ModQ<3329>{-1}.value() == 3328);

  const auto a = ModQ<17>{15};
  const auto b = ModQ<17>{5};
  assert((a + b).value() == 3);
  assert((a - b).value() == 10);
  assert((a * b).value() == 7);
  assert((-b).value() == 12);

  assert(ModQ<17>{8}.centered() == 8);
  assert(ModQ<17>{9}.centered() == -8);
}

