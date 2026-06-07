#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "pqcore/dsa_helpers.hpp"

template <typename Fn>
void assert_throws_invalid_argument(Fn&& fn) {
  bool threw = false;
  try {
    fn();
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);
}

std::vector<std::uint8_t> challenge_fixture_bytes(std::size_t tau) {
  std::vector<std::uint8_t> bytes(8, 0);
  for (std::size_t offset = 0; offset < tau; ++offset) {
    bytes.push_back(static_cast<std::uint8_t>(256 - tau + offset));
  }
  return bytes;
}

int hamming_weight_pm_one(const pqcore::MlDsaPolynomial& polynomial) {
  int weight = 0;
  for (std::size_t index = 0; index < pqcore::MlDsaPolynomial::degree(); ++index) {
    const auto centered = polynomial.coeff_at(index).centered();
    assert(centered == -1 || centered == 0 || centered == 1);
    if (centered != 0) {
      ++weight;
    }
  }
  return weight;
}

void assert_rounding_invariants(pqcore::MlDsaParameterSet parameter_set) {
  const auto selected = pqcore::params(parameter_set);
  const std::vector<std::int64_t> values{
      0,
      1,
      selected.gamma2,
      static_cast<std::int64_t>(selected.gamma2) + 1,
      selected.q - 1,
      selected.q,
      static_cast<std::int64_t>(selected.q) + selected.gamma2 + 1,
      -1,
  };

  const auto power2_base = pqcore::ml_dsa_power2round_base(selected);
  const auto alpha = pqcore::ml_dsa_decompose_base(selected);
  const auto high_modulus = pqcore::ml_dsa_high_bits_modulus(selected);
  assert((selected.q - 1) % alpha == 0);
  assert(high_modulus == 44 || high_modulus == 16);

  for (const auto value : values) {
    const auto canonical = pqcore::ml_dsa_positive_mod(value, selected.q);
    const auto power = pqcore::ml_dsa_power2round(selected, value);
    assert(pqcore::ml_dsa_positive_mod(
               static_cast<std::int64_t>(power.high) * power2_base + power.low,
               selected.q) == canonical);
    assert(power.low >= -(power2_base / 2) + 1);
    assert(power.low <= power2_base / 2);

    const auto decomposition = pqcore::ml_dsa_decompose(selected, value);
    assert(pqcore::ml_dsa_positive_mod(
               static_cast<std::int64_t>(decomposition.high) * alpha + decomposition.low,
               selected.q) == canonical);
    assert(decomposition.low >= -selected.gamma2);
    assert(decomposition.low <= selected.gamma2);
    assert(decomposition.high >= 0);
    assert(decomposition.high < high_modulus);
    assert(pqcore::ml_dsa_high_bits(selected, value) == decomposition.high);
    assert(pqcore::ml_dsa_low_bits(selected, value) == decomposition.low);
  }

  const auto power_boundary =
      pqcore::ml_dsa_power2round(selected, (power2_base / 2) + 1);
  assert(power_boundary.high == 1);
  assert(power_boundary.low == -(power2_base / 2) + 1);

  const auto special = pqcore::ml_dsa_decompose(selected, selected.q - 1);
  assert(special.high == 0);
  assert(special.low == -1);

  const auto hint_boundary = pqcore::ml_dsa_decompose(selected, selected.gamma2);
  assert(hint_boundary.high == 0);
  assert(hint_boundary.low == selected.gamma2);
  assert(!pqcore::ml_dsa_make_hint(selected, 1, 0));
  assert(pqcore::ml_dsa_make_hint(selected, 1, selected.gamma2));
  assert(pqcore::ml_dsa_use_hint(selected, false, selected.gamma2) ==
         pqcore::ml_dsa_high_bits(selected, selected.gamma2));
  assert(pqcore::ml_dsa_use_hint(selected, true, selected.gamma2) == 1);
  assert(pqcore::ml_dsa_use_hint(selected, true, 0) == high_modulus - 1);
  assert(pqcore::ml_dsa_use_hint(selected, true, alpha) == 0);
}

void assert_challenge_invariants(pqcore::MlDsaParameterSet parameter_set) {
  const auto selected = pqcore::params(parameter_set);
  assert(pqcore::ml_dsa_challenge_seed_bytes(selected) ==
         selected.lambda_bits / 4);

  const auto bytes = challenge_fixture_bytes(selected.tau);
  const auto challenge = pqcore::ml_dsa_challenge_polynomial_from_bytes(parameter_set, bytes);
  assert(hamming_weight_pm_one(challenge) == static_cast<int>(selected.tau));
  for (std::size_t index = 0; index < 256 - selected.tau; ++index) {
    assert(challenge.coeff_at(index).centered() == 0);
  }
  for (std::size_t index = 256 - selected.tau; index < 256; ++index) {
    assert(challenge.coeff_at(index).centered() == 1);
  }

  assert_throws_invalid_argument([&] {
    (void)pqcore::ml_dsa_challenge_polynomial_from_bytes(
        parameter_set, std::vector<std::uint8_t>(8));
  });
}

int main() {
  static_assert(pqcore::ml_dsa_positive_mod(-1, pqcore::kMlDsaQ) == pqcore::kMlDsaQ - 1);
  static_assert(pqcore::ml_dsa_centered_mod(4096, 8192) == 4096);
  static_assert(pqcore::ml_dsa_centered_mod(4097, 8192) == -4095);
  static_assert(pqcore::ml_dsa_power2round(pqcore::MlDsaParameterSet::MlDsa44, 4097).high == 1);
  static_assert(pqcore::ml_dsa_power2round(pqcore::MlDsaParameterSet::MlDsa44, 4097).low ==
                -4095);

  assert_rounding_invariants(pqcore::MlDsaParameterSet::MlDsa44);
  assert_rounding_invariants(pqcore::MlDsaParameterSet::MlDsa65);
  assert_rounding_invariants(pqcore::MlDsaParameterSet::MlDsa87);

  assert_challenge_invariants(pqcore::MlDsaParameterSet::MlDsa44);
  assert_challenge_invariants(pqcore::MlDsaParameterSet::MlDsa65);
  assert_challenge_invariants(pqcore::MlDsaParameterSet::MlDsa87);

  assert_throws_invalid_argument([] { (void)pqcore::ml_dsa_positive_mod(1, 0); });
  assert_throws_invalid_argument([] { (void)pqcore::ml_dsa_centered_mod(1, 3); });
}
