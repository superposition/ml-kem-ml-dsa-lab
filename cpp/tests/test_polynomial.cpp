#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "pqcore/polynomial.hpp"

template <typename Poly>
void assert_zero(const Poly& poly) {
  for (std::size_t index = 0; index < Poly::degree(); ++index) {
    assert(poly.coeff_at(index).value() == 0);
  }
}

template <typename Poly>
void assert_same_coefficients(const Poly& lhs, const Poly& rhs) {
  for (std::size_t index = 0; index < Poly::degree(); ++index) {
    assert(lhs.coeff_at(index).value() == rhs.coeff_at(index).value());
  }
}

int main() {
  using Poly = pqcore::Polynomial<4, 17>;

  static_assert(Poly::degree() == 4);
  static_assert(Poly::modulus() == 17);
  static_assert(pqcore::MlKemPolynomial::degree() == 256);
  static_assert(pqcore::MlKemPolynomial::modulus() == 3329);
  static_assert(pqcore::MlDsaPolynomial::degree() == 256);
  static_assert(pqcore::MlDsaPolynomial::modulus() == 8380417);
  static_assert(!std::is_same_v<pqcore::MlKemPolynomial, pqcore::MlDsaPolynomial>);

  assert_zero(Poly::zero());

  const auto normalized = Poly{{17, -1, 18, -18}};
  assert(normalized.coeff_at(0).value() == 0);
  assert(normalized.coeff_at(1).value() == 16);
  assert(normalized.coeff_at(2).value() == 1);
  assert(normalized.coeff_at(3).value() == 16);

  const auto a = Poly{{1, 2, 3, 4}};
  const auto b = Poly{{4, 3, 2, 1}};
  const auto sum = a.add(b);
  const auto diff = a.sub(b);

  assert(sum.coeff_at(0).value() == 5);
  assert(sum.coeff_at(1).value() == 5);
  assert(sum.coeff_at(2).value() == 5);
  assert(sum.coeff_at(3).value() == 5);
  assert(diff.coeff_at(0).value() == 14);
  assert(diff.coeff_at(1).value() == 16);
  assert(diff.coeff_at(2).value() == 1);
  assert(diff.coeff_at(3).value() == 3);

  const auto x3 = Poly{{0, 0, 0, 1}};
  const auto x = Poly{{0, 1, 0, 0}};
  const auto product = x3.schoolbook_mul_negacyclic(x);
  assert(product.coeff_at(0).value() == 16);
  assert(product.coeff_at(1).value() == 0);
  assert(product.coeff_at(2).value() == 0);
  assert(product.coeff_at(3).value() == 0);

  assert_zero(a.schoolbook_mul_negacyclic(Poly::zero()));
  assert_zero(Poly::zero().schoolbook_mul_negacyclic(a));
  assert_same_coefficients(a.schoolbook_mul_negacyclic(Poly::one()), a);
  assert_same_coefficients(Poly::one().schoolbook_mul_negacyclic(a), a);

  const auto distributed_left = a.schoolbook_mul_negacyclic(b.add(Poly::one()));
  const auto distributed_right = a.schoolbook_mul_negacyclic(b).add(a);
  assert_same_coefficients(distributed_left, distributed_right);

  std::array<std::int32_t, 256> real_x_coefficients{};
  real_x_coefficients[1] = 1;
  std::array<std::int32_t, 256> real_x255_coefficients{};
  real_x255_coefficients[255] = 1;

  const auto real_product = pqcore::MlKemPolynomial{real_x255_coefficients}
                                .schoolbook_mul_negacyclic(
                                    pqcore::MlKemPolynomial{real_x_coefficients});
  assert(real_product.coeff_at(0).value() == 3328);
  for (std::size_t index = 1; index < pqcore::MlKemPolynomial::degree(); ++index) {
    assert(real_product.coeff_at(index).value() == 0);
  }

  std::array<std::int32_t, 256> dsa_coefficients{};
  dsa_coefficients[0] = 8380416;
  dsa_coefficients[1] = 2;
  dsa_coefficients[255] = -1;
  const auto dsa_poly = pqcore::MlDsaPolynomial{dsa_coefficients};
  assert(dsa_poly.coeff_at(0).value() == 8380416);
  assert(dsa_poly.coeff_at(1).value() == 2);
  assert(dsa_poly.coeff_at(255).value() == 8380416);
  assert_same_coefficients(dsa_poly.schoolbook_mul_negacyclic(pqcore::MlDsaPolynomial::one()),
                           dsa_poly);
  assert_zero(dsa_poly.schoolbook_mul_negacyclic(pqcore::MlDsaPolynomial::zero()));
}
