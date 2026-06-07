#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "pqcore/sampling.hpp"

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

std::vector<std::uint8_t> encode_ml_kem_sample_ntt_pair(std::int32_t d1,
                                                        std::int32_t d2) {
  return {
      static_cast<std::uint8_t>(d1 & 0xff),
      static_cast<std::uint8_t>(((d1 >> 8) & 0x0f) | ((d2 & 0x0f) << 4)),
      static_cast<std::uint8_t>((d2 >> 4) & 0xff),
  };
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

int main() {
  static_assert(pqcore::kDeterministicTestSamplingEnabled);

  pqcore::MlKemSeed seed{};
  for (std::size_t index = 0; index < seed.size(); ++index) {
    seed[index] = static_cast<std::uint8_t>(index);
  }

  const auto expanded = pqcore::deterministic_test_expand(seed, "fixture", 7, 16);
  const std::vector<std::uint8_t> expected_expanded{
      240, 1, 28, 230, 209, 152, 45, 10, 114, 119, 225, 12, 124, 142, 167, 51};
  assert(expanded == expected_expanded);
  assert(pqcore::deterministic_test_expand(seed, "fixture", 7, 16) == expected_expanded);
  assert(pqcore::deterministic_test_expand(seed, "fixture", 8, 16) != expected_expanded);
  assert(pqcore::deterministic_test_expand(seed, "other-domain", 7, 16) != expected_expanded);

  const auto prf_eta2 = pqcore::ml_kem_prf_test(seed, 9, 2);
  const std::vector<std::uint8_t> expected_prf_prefix{
      22, 65, 91, 184, 153, 86, 65, 243, 23, 180, 197, 11, 18, 58, 43, 10};
  assert(prf_eta2.size() == 128);
  assert(std::vector<std::uint8_t>(prf_eta2.begin(), prf_eta2.begin() + 16) ==
         expected_prf_prefix);
  assert(pqcore::ml_kem_prf_test(seed, 9, 3).size() == 192);
  assert(pqcore::ml_kem_prf_test(seed, 10, 2) != prf_eta2);
  assert_throws_invalid_argument([&] { (void)pqcore::ml_kem_prf_test(seed, 9, 4); });

  std::vector<std::uint8_t> sample_ntt_bytes;
  auto rejected = encode_ml_kem_sample_ntt_pair(4095, 4095);
  sample_ntt_bytes.insert(sample_ntt_bytes.end(), rejected.begin(), rejected.end());
  for (std::int32_t value = 0; value < 256; value += 2) {
    const auto encoded = encode_ml_kem_sample_ntt_pair(value, value + 1);
    sample_ntt_bytes.insert(sample_ntt_bytes.end(), encoded.begin(), encoded.end());
  }

  const auto sample_ntt = pqcore::ml_kem_sample_ntt_from_bytes(sample_ntt_bytes);
  for (std::size_t index = 0; index < pqcore::MlKemNttPolynomial::degree(); ++index) {
    assert(sample_ntt.coeff_at(index).value() == static_cast<std::int32_t>(index));
  }
  assert_throws_invalid_argument([] {
    (void)pqcore::ml_kem_sample_ntt_from_bytes(std::vector<std::uint8_t>{0, 1});
  });

  pqcore::MlKemSampleNttSeed xof_seed{};
  for (std::size_t index = 0; index < 32; ++index) {
    xof_seed[index] = static_cast<std::uint8_t>(index);
  }
  xof_seed[32] = 1;
  xof_seed[33] = 2;
  const auto xof_bytes = pqcore::ml_kem_xof_test(xof_seed, 16);
  const std::vector<std::uint8_t> expected_xof_prefix{
      41, 16, 175, 38, 6, 251, 119, 42, 193, 122, 196, 191, 20, 227, 211, 231};
  assert(xof_bytes == expected_xof_prefix);
  const auto sampled_from_hook = pqcore::ml_kem_sample_ntt_test(xof_seed);
  for (std::size_t index = 0; index < pqcore::MlKemNttPolynomial::degree(); ++index) {
    assert(sampled_from_hook.coeff_at(index).value() >= 0);
    assert(sampled_from_hook.coeff_at(index).value() < pqcore::MlKemField::modulus());
  }
  auto changed_xof_seed = xof_seed;
  changed_xof_seed[32] = 2;
  bool saw_difference = false;
  const auto changed_sample = pqcore::ml_kem_sample_ntt_test(changed_xof_seed);
  for (std::size_t index = 0; index < pqcore::MlKemNttPolynomial::degree(); ++index) {
    saw_difference =
        saw_difference || sampled_from_hook.coeff_at(index).value() !=
                              changed_sample.coeff_at(index).value();
  }
  assert(saw_difference);

  std::vector<std::uint8_t> cbd_bytes_eta2(128, 0);
  auto cbd = pqcore::ml_kem_sample_poly_cbd(2, cbd_bytes_eta2);
  for (std::size_t index = 0; index < pqcore::MlKemPolynomial::degree(); ++index) {
    assert(cbd.coeff_at(index).centered() == 0);
  }
  cbd_bytes_eta2[0] = 0x01;
  cbd = pqcore::ml_kem_sample_poly_cbd(2, cbd_bytes_eta2);
  assert(cbd.coeff_at(0).centered() == 1);
  cbd_bytes_eta2[0] = 0x04;
  cbd = pqcore::ml_kem_sample_poly_cbd(2, cbd_bytes_eta2);
  assert(cbd.coeff_at(0).centered() == -1);
  const auto cbd_from_prf = pqcore::ml_kem_sample_poly_cbd(2, prf_eta2);
  for (std::size_t index = 0; index < pqcore::MlKemPolynomial::degree(); ++index) {
    assert(cbd_from_prf.coeff_at(index).centered() >= -2);
    assert(cbd_from_prf.coeff_at(index).centered() <= 2);
  }
  assert_throws_invalid_argument([] {
    (void)pqcore::ml_kem_sample_poly_cbd(2, std::vector<std::uint8_t>(127));
  });

  static_assert(pqcore::ml_dsa_coeff_from_three_bytes(0, 0, 0).value() == 0);
  static_assert(pqcore::ml_dsa_coeff_from_three_bytes(0xff, 0xff, 0xff) == std::nullopt);
  static_assert(pqcore::ml_dsa_coeff_from_three_bytes(0, 0, 128).value() == 0);
  static_assert(pqcore::ml_dsa_coeff_from_half_byte(0, 2).value() == 2);
  static_assert(pqcore::ml_dsa_coeff_from_half_byte(4, 2).value() == -2);
  static_assert(pqcore::ml_dsa_coeff_from_half_byte(15, 2) == std::nullopt);
  static_assert(pqcore::ml_dsa_coeff_from_half_byte(0, 4).value() == 4);
  static_assert(pqcore::ml_dsa_coeff_from_half_byte(8, 4).value() == -4);
  static_assert(pqcore::ml_dsa_coeff_from_half_byte(9, 4) == std::nullopt);

  std::vector<std::uint8_t> bounded_bytes(128, 0x10);
  const auto bounded = pqcore::ml_dsa_rej_bounded_poly_from_bytes(2, bounded_bytes);
  for (std::size_t index = 0; index < pqcore::MlDsaPolynomial::degree(); ++index) {
    assert(bounded.coeff_at(index).centered() >= -2);
    assert(bounded.coeff_at(index).centered() <= 2);
  }
  assert_throws_invalid_argument([] {
    (void)pqcore::ml_dsa_rej_bounded_poly_from_bytes(2, std::vector<std::uint8_t>{0xff});
  });

  std::vector<std::uint8_t> ball_bytes(8, 0);
  ball_bytes[0] = 0b00001010;
  ball_bytes.push_back(252);
  ball_bytes.push_back(253);
  ball_bytes.push_back(254);
  ball_bytes.push_back(255);
  const auto ball = pqcore::ml_dsa_sample_in_ball_from_bytes(4, ball_bytes);
  assert(hamming_weight_pm_one(ball) == 4);
  assert(ball.coeff_at(252).centered() == 1);
  assert(ball.coeff_at(253).centered() == -1);
  assert(ball.coeff_at(254).centered() == 1);
  assert(ball.coeff_at(255).centered() == -1);
  assert_throws_invalid_argument([] {
    (void)pqcore::ml_dsa_sample_in_ball_from_bytes(65, std::vector<std::uint8_t>(100));
  });
}
