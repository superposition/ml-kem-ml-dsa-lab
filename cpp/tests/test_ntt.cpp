#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "pqcore/ntt.hpp"

template <typename Poly>
void assert_zero(const Poly& poly) {
  for (std::size_t index = 0; index < Poly::degree(); ++index) {
    assert(poly.coeff_at(index).value() == 0);
  }
}

void assert_same_coefficients(const pqcore::MlKemPolynomial& lhs,
                              const pqcore::MlKemPolynomial& rhs) {
  for (std::size_t index = 0; index < pqcore::MlKemPolynomial::degree(); ++index) {
    assert(lhs.coeff_at(index).value() == rhs.coeff_at(index).value());
  }
}

pqcore::MlKemPolynomial make_sample_polynomial(std::int32_t seed) {
  std::array<std::int32_t, 256> coefficients{};
  for (std::size_t index = 0; index < coefficients.size(); ++index) {
    coefficients[index] =
        static_cast<std::int32_t>((seed + (17 * index) + (index * index)) % 3329);
  }
  return pqcore::MlKemPolynomial{coefficients};
}

int main() {
  static_assert(!std::is_convertible_v<pqcore::MlKemPolynomial, pqcore::MlKemNttPolynomial>);
  static_assert(!std::is_convertible_v<pqcore::MlKemNttPolynomial, pqcore::MlKemPolynomial>);
  static_assert(pqcore::MlKemNttPolynomial::degree() == 256);
  static_assert(pqcore::MlKemNttPolynomial::modulus() == 3329);
  static_assert(pqcore::bit_reverse_7(0) == 0);
  static_assert(pqcore::bit_reverse_7(1) == 64);
  static_assert(pqcore::bit_reverse_7(2) == 32);
  static_assert(pqcore::bit_reverse_7(127) == 127);
  static_assert(pqcore::ml_kem_ntt_zeta(0).value() == 1);
  static_assert(pqcore::ml_kem_ntt_zeta(1).value() == 1729);
  static_assert(pqcore::ml_kem_ntt_zeta(2).value() == 2580);
  static_assert(pqcore::ml_kem_ntt_zeta(127).value() == 2154);
  static_assert(pqcore::ml_kem_ntt_gamma(0).value() == 17);
  static_assert(pqcore::ml_kem_ntt_gamma(1).value() == 3312);
  static_assert(pqcore::ml_kem_ntt_gamma(2).value() == 2761);
  static_assert(pqcore::ml_kem_ntt_gamma(3).value() == 568);

  assert_zero(pqcore::ml_kem_ntt(pqcore::MlKemPolynomial::zero()));
  assert_zero(pqcore::ml_kem_inverse_ntt(pqcore::MlKemNttPolynomial{}));

  const auto one_ntt = pqcore::ml_kem_ntt(pqcore::MlKemPolynomial::one());
  for (std::size_t block = 0; block < 128; ++block) {
    assert(one_ntt.coeff_at(2 * block).value() == 1);
    assert(one_ntt.coeff_at((2 * block) + 1).value() == 0);
  }
  assert_same_coefficients(pqcore::ml_kem_inverse_ntt(one_ntt), pqcore::MlKemPolynomial::one());

  const auto sample = make_sample_polynomial(23);
  assert_same_coefficients(pqcore::ml_kem_inverse_ntt(pqcore::ml_kem_ntt(sample)), sample);

  const auto lhs = make_sample_polynomial(5);
  const auto rhs = make_sample_polynomial(91);
  const auto ntt_product =
      pqcore::ml_kem_inverse_ntt(pqcore::ml_kem_ntt_multiply(pqcore::ml_kem_ntt(lhs),
                                                             pqcore::ml_kem_ntt(rhs)));
  const auto schoolbook_product = lhs.schoolbook_mul_negacyclic(rhs);
  assert_same_coefficients(ntt_product, schoolbook_product);

  const auto base_product = pqcore::ml_kem_ntt_base_case_multiply(
      pqcore::MlKemField{3},
      pqcore::MlKemField{4},
      pqcore::MlKemField{5},
      pqcore::MlKemField{6},
      pqcore::MlKemField{17});
  assert(base_product[0].value() == 423);
  assert(base_product[1].value() == 38);
}

