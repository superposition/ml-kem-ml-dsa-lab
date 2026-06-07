#include <array>
#include <cassert>

#include "pqcore/polynomial.hpp"

int main() {
  using Poly = pqcore::Polynomial<4, 17>;

  const auto a = Poly{{1, 2, 3, 4}};
  const auto b = Poly{{4, 3, 2, 1}};
  const auto sum = a.add(b);
  const auto diff = a.sub(b);

  assert(sum.coeff_at(0).value() == 5);
  assert(sum.coeff_at(3).value() == 5);
  assert(diff.coeff_at(0).value() == 14);
  assert(diff.coeff_at(1).value() == 16);

  const auto x3 = Poly{{0, 0, 0, 1}};
  const auto x = Poly{{0, 1, 0, 0}};
  const auto product = x3.schoolbook_mul_negacyclic(x);
  assert(product.coeff_at(0).value() == 16);
  assert(product.coeff_at(1).value() == 0);
}

