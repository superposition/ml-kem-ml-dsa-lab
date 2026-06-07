#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "pqcore/kem_internal.hpp"

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

std::vector<std::uint8_t> first_bytes(const std::vector<std::uint8_t>& bytes,
                                      std::size_t count) {
  return std::vector<std::uint8_t>(
      bytes.begin(), bytes.begin() + static_cast<std::ptrdiff_t>(count));
}

std::vector<std::uint8_t> first_bytes(const pqcore::SharedSecret& secret,
                                      std::size_t count) {
  return std::vector<std::uint8_t>(
      secret.bytes.begin(), secret.bytes.begin() + static_cast<std::ptrdiff_t>(count));
}

int main() {
  static_assert(pqcore::kDeterministicTestSamplingEnabled);
  static_assert(pqcore::kMlKem512DecapsulationKeyBytes == 1632);

  pqcore::MlKemSeed keygen_seed{};
  pqcore::MlKemSeed rejection_seed{};
  pqcore::MlKemSeed message_seed{};
  for (std::size_t index = 0; index < keygen_seed.size(); ++index) {
    keygen_seed[index] = static_cast<std::uint8_t>(index);
    rejection_seed[index] = static_cast<std::uint8_t>(0xa0u + index);
    message_seed[index] = 0;
  }

  const auto pke_keypair = pqcore::ml_kem_pke_keygen_512_test(keygen_seed);
  const auto keypair =
      pqcore::ml_kem_keygen_internal_512_test(keygen_seed, rejection_seed);
  assert(keypair.public_key.size() == pqcore::kMlKem512PublicKeyBytes);
  assert(keypair.secret_key.size() == pqcore::kMlKem512DecapsulationKeyBytes);
  assert(keypair.secret_key.size() ==
         pqcore::params(pqcore::MlKemParameterSet::MlKem512).secret_key_bytes);
  assert(first_bytes(keypair.secret_key, pqcore::kMlKem512SecretKeyBytes) ==
         pke_keypair.decryption_key);
  assert(std::vector<std::uint8_t>(
             keypair.secret_key.begin() + static_cast<std::ptrdiff_t>(
                                            pqcore::kMlKem512SecretKeyBytes),
             keypair.secret_key.begin() + static_cast<std::ptrdiff_t>(
                 pqcore::kMlKem512SecretKeyBytes + pqcore::kMlKem512PublicKeyBytes)) ==
         keypair.public_key);

  const auto public_key_hash = pqcore::ml_kem_hash_h_test(keypair.public_key);
  assert(std::vector<std::uint8_t>(
             keypair.secret_key.begin() + static_cast<std::ptrdiff_t>(
                 pqcore::kMlKem512SecretKeyBytes + pqcore::kMlKem512PublicKeyBytes),
             keypair.secret_key.begin() + static_cast<std::ptrdiff_t>(
                 pqcore::kMlKem512SecretKeyBytes + pqcore::kMlKem512PublicKeyBytes +
                 pqcore::kMlKemSeedBytes)) ==
         std::vector<std::uint8_t>(public_key_hash.begin(), public_key_hash.end()));
  assert(std::vector<std::uint8_t>(
             keypair.secret_key.end() - static_cast<std::ptrdiff_t>(pqcore::kMlKemSeedBytes),
             keypair.secret_key.end()) ==
         std::vector<std::uint8_t>(rejection_seed.begin(), rejection_seed.end()));

  const auto keypair_again =
      pqcore::ml_kem_keygen_internal_512_test(keygen_seed, rejection_seed);
  assert(keypair_again.public_key == keypair.public_key);
  assert(keypair_again.secret_key == keypair.secret_key);

  const auto encapsulation =
      pqcore::ml_kem_encaps_internal_512_test(keypair.public_key, message_seed);
  const auto encapsulation_again =
      pqcore::ml_kem_encaps_internal_512_test(keypair.public_key, message_seed);
  assert(encapsulation.ciphertext.bytes.size() == pqcore::kMlKem512CiphertextBytes);
  assert(encapsulation.ciphertext.bytes ==
         encapsulation_again.ciphertext.bytes);
  assert(encapsulation.shared_secret.bytes ==
         encapsulation_again.shared_secret.bytes);
  assert(pqcore::ml_kem_decaps_internal_512_test(
             keypair.secret_key, encapsulation.ciphertext.bytes)
             .bytes == encapsulation.shared_secret.bytes);

  auto corrupted_ciphertext = encapsulation.ciphertext.bytes;
  corrupted_ciphertext[0] ^= 0x01u;
  const auto corrupted_secret =
      pqcore::ml_kem_decaps_internal_512_test(keypair.secret_key, corrupted_ciphertext);
  const auto corrupted_secret_again =
      pqcore::ml_kem_decaps_internal_512_test(keypair.secret_key, corrupted_ciphertext);
  assert(corrupted_secret.bytes == corrupted_secret_again.bytes);
  assert(corrupted_secret.bytes != encapsulation.shared_secret.bytes);

  const std::vector<std::uint8_t> expected_public_key_prefix{
      34, 149, 121, 190, 36, 122, 162, 67, 133, 253, 161, 47, 111, 245, 35, 231};
  const std::vector<std::uint8_t> expected_secret_key_prefix{
      185, 18, 12, 45, 182, 163, 105, 73, 31, 158, 201, 28, 119, 227, 51, 187};
  const std::vector<std::uint8_t> expected_public_key_hash_prefix{
      37, 61, 0, 242, 94, 51, 53, 148, 60, 49, 52, 252, 22, 164, 226, 206};
  const std::vector<std::uint8_t> expected_ciphertext_prefix{
      121, 8, 58, 105, 70, 33, 200, 148, 162, 246, 255, 125, 246, 243, 127, 186};
  const std::vector<std::uint8_t> expected_shared_secret_prefix{
      114, 33, 104, 72, 0, 63, 247, 254, 231, 65, 4, 203, 130, 80, 67, 216};
  const std::vector<std::uint8_t> expected_fallback_secret_prefix{
      21, 187, 67, 59, 125, 241, 54, 198, 147, 55, 7, 134, 202, 127, 98, 107};
  assert(first_bytes(keypair.public_key, 16) == expected_public_key_prefix);
  assert(first_bytes(keypair.secret_key, 16) == expected_secret_key_prefix);
  assert(std::vector<std::uint8_t>(public_key_hash.begin(), public_key_hash.begin() + 16) ==
         expected_public_key_hash_prefix);
  assert(first_bytes(encapsulation.ciphertext.bytes, 16) == expected_ciphertext_prefix);
  assert(first_bytes(encapsulation.shared_secret, 16) == expected_shared_secret_prefix);
  assert(first_bytes(corrupted_secret, 16) == expected_fallback_secret_prefix);

  const auto equal_mask = pqcore::ml_kem_constant_time_equal_mask(
      encapsulation.ciphertext.bytes, encapsulation.ciphertext.bytes);
  const auto unequal_mask = pqcore::ml_kem_constant_time_equal_mask(
      encapsulation.ciphertext.bytes, corrupted_ciphertext);
  assert(equal_mask == 0xffu);
  assert(unequal_mask == 0x00u);

  assert_throws_invalid_argument([&] {
    (void)pqcore::ml_kem_encaps_internal_512_test(
        std::vector<std::uint8_t>(pqcore::kMlKem512PublicKeyBytes - 1), message_seed);
  });
  assert_throws_invalid_argument([&] {
    (void)pqcore::ml_kem_decaps_internal_512_test(
        std::vector<std::uint8_t>(pqcore::kMlKem512DecapsulationKeyBytes - 1),
        encapsulation.ciphertext.bytes);
  });
  assert_throws_invalid_argument([&] {
    (void)pqcore::ml_kem_decaps_internal_512_test(
        keypair.secret_key, std::vector<std::uint8_t>(pqcore::kMlKem512CiphertextBytes - 1));
  });
  assert_throws_invalid_argument([&] {
    (void)pqcore::ml_kem_constant_time_equal_mask(
        std::vector<std::uint8_t>{1}, std::vector<std::uint8_t>{1, 2});
  });
}
