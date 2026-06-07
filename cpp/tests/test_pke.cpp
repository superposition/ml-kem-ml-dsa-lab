#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "pqcore/pke.hpp"

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

bool same_ntt_polynomial(const pqcore::MlKemNttPolynomial& lhs,
                         const pqcore::MlKemNttPolynomial& rhs) {
  for (std::size_t index = 0; index < pqcore::MlKemNttPolynomial::degree(); ++index) {
    if (lhs.coeff_at(index).value() != rhs.coeff_at(index).value()) {
      return false;
    }
  }
  return true;
}

int main() {
  static_assert(pqcore::kDeterministicTestSamplingEnabled);
  static_assert(pqcore::kMlKem512K == 2);

  pqcore::MlKemSeed keygen_seed{};
  pqcore::MlKemSeed coins{};
  for (std::size_t index = 0; index < keygen_seed.size(); ++index) {
    keygen_seed[index] = static_cast<std::uint8_t>(index);
    coins[index] = static_cast<std::uint8_t>(255 - index);
  }

  const auto expanded = pqcore::ml_kem_pke_expand_keygen_seeds_512_test(keygen_seed);
  const auto matrix = pqcore::ml_kem_pke_generate_matrix_512_test(expanded[0]);
  const auto matrix_again = pqcore::ml_kem_pke_generate_matrix_512_test(expanded[0]);
  assert(matrix.size() == pqcore::kMlKem512K);
  assert(matrix[0].size() == pqcore::kMlKem512K);
  assert(same_ntt_polynomial(matrix[0][0], matrix_again[0][0]));
  assert(!same_ntt_polynomial(matrix[0][0], matrix[0][1]));
  assert(!same_ntt_polynomial(matrix[0][0], matrix[1][0]));

  const auto secret = pqcore::ml_kem_pke_sample_noise_vector_512_test(expanded[1], 3, 0);
  assert(secret.size() == pqcore::kMlKem512K);
  for (const auto& polynomial : secret) {
    for (std::size_t index = 0; index < pqcore::MlKemPolynomial::degree(); ++index) {
      assert(polynomial.coeff_at(index).centered() >= -3);
      assert(polynomial.coeff_at(index).centered() <= 3);
    }
  }

  const auto keypair = pqcore::ml_kem_pke_keygen_512_test(keygen_seed);
  assert(keypair.encryption_key.size() == pqcore::kMlKem512PublicKeyBytes);
  assert(keypair.decryption_key.size() == pqcore::kMlKem512SecretKeyBytes);
  const std::vector<std::uint8_t> expected_encryption_key_prefix{
      34, 149, 121, 190, 36, 122, 162, 67, 133, 253, 161, 47, 111, 245, 35, 231};
  const std::vector<std::uint8_t> expected_decryption_key_prefix{
      185, 18, 12, 45, 182, 163, 105, 73, 31, 158, 201, 28, 119, 227, 51, 187};
  assert(std::vector<std::uint8_t>(keypair.encryption_key.begin(),
                                   keypair.encryption_key.begin() + 16) ==
         expected_encryption_key_prefix);
  assert(std::vector<std::uint8_t>(keypair.decryption_key.begin(),
                                   keypair.decryption_key.begin() + 16) ==
         expected_decryption_key_prefix);

  const std::vector<std::uint8_t> message(32, 0);
  const auto ciphertext = pqcore::ml_kem_pke_encrypt_512_test(
      keypair.encryption_key, message, coins);
  assert(ciphertext.bytes.size() == pqcore::kMlKem512CiphertextBytes);
  const std::vector<std::uint8_t> expected_ciphertext_prefix{
      145, 204, 62, 127, 46, 193, 123, 249, 140, 194, 32, 141, 118, 72, 150, 222};
  assert(std::vector<std::uint8_t>(ciphertext.bytes.begin(), ciphertext.bytes.begin() + 16) ==
         expected_ciphertext_prefix);
  assert(pqcore::ml_kem_pke_decrypt_512(keypair.decryption_key, ciphertext.bytes) == message);

  assert_throws_invalid_argument([&] {
    (void)pqcore::ml_kem_pke_encrypt_512_test(
        std::vector<std::uint8_t>(pqcore::kMlKem512PublicKeyBytes - 1), message, coins);
  });
  assert_throws_invalid_argument([&] {
    (void)pqcore::ml_kem_pke_encrypt_512_test(
        keypair.encryption_key, std::vector<std::uint8_t>(31), coins);
  });
  assert_throws_invalid_argument([&] {
    (void)pqcore::ml_kem_pke_decrypt_512(
        std::vector<std::uint8_t>(pqcore::kMlKem512SecretKeyBytes - 1), ciphertext.bytes);
  });
  assert_throws_invalid_argument([&] {
    (void)pqcore::ml_kem_pke_decrypt_512(
        keypair.decryption_key, std::vector<std::uint8_t>(pqcore::kMlKem512CiphertextBytes - 1));
  });
}
