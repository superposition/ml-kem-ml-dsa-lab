#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

#include "pqcore/kem.hpp"
#include "pqcore/pke.hpp"

namespace pqcore {

inline constexpr std::size_t kMlKemSharedSecretBytes = 32;
inline constexpr std::size_t kMlKem512DecapsulationKeyBytes =
    kMlKem512SecretKeyBytes + kMlKem512PublicKeyBytes + 2 * kMlKemSeedBytes;

struct MlKemInternalEncapsulation {
  MlKemCiphertext ciphertext;
  SharedSecret shared_secret;
};

struct MlKemGOutput {
  SharedSecret shared_secret;
  MlKemSeed coins;
};

[[nodiscard]] inline std::vector<std::uint8_t> ml_kem_concat(
    std::span<const std::uint8_t> lhs,
    std::span<const std::uint8_t> rhs) {
  std::vector<std::uint8_t> output;
  output.reserve(lhs.size() + rhs.size());
  output.insert(output.end(), lhs.begin(), lhs.end());
  output.insert(output.end(), rhs.begin(), rhs.end());
  return output;
}

[[nodiscard]] inline std::uint8_t ml_kem_constant_time_equal_mask(
    std::span<const std::uint8_t> lhs,
    std::span<const std::uint8_t> rhs) {
  if (lhs.size() != rhs.size()) {
    throw std::invalid_argument{"constant-time comparison requires equal lengths"};
  }

  std::uint32_t diff = 0;
  for (std::size_t index = 0; index < lhs.size(); ++index) {
    diff |= static_cast<std::uint32_t>(lhs[index] ^ rhs[index]);
  }
  const auto is_zero = ((diff | (0u - diff)) >> 31) ^ 1u;
  return static_cast<std::uint8_t>(0u - is_zero);
}

[[nodiscard]] inline SharedSecret ml_kem_select_shared_secret(
    const SharedSecret& candidate,
    const SharedSecret& fallback,
    std::uint8_t use_candidate_mask) {
  SharedSecret selected{};
  const auto use_fallback_mask = static_cast<std::uint8_t>(~use_candidate_mask);
  for (std::size_t index = 0; index < selected.bytes.size(); ++index) {
    selected.bytes[index] = static_cast<std::uint8_t>(
        (candidate.bytes[index] & use_candidate_mask) |
        (fallback.bytes[index] & use_fallback_mask));
  }
  return selected;
}

[[nodiscard]] inline MlKemSeed ml_kem_hash_h(std::span<const std::uint8_t> input) {
  const auto digest = sha3_256(input);
  MlKemSeed output{};
  std::copy(digest.begin(), digest.end(), output.begin());
  return output;
}

[[nodiscard]] inline MlKemGOutput ml_kem_g(std::span<const std::uint8_t> input) {
  const auto digest = sha3_512(input);
  MlKemGOutput output{};
  std::copy_n(digest.begin(), kMlKemSharedSecretBytes, output.shared_secret.bytes.begin());
  std::copy_n(digest.begin() + kMlKemSharedSecretBytes, kMlKemSeedBytes, output.coins.begin());
  return output;
}

[[nodiscard]] inline SharedSecret ml_kem_j(const MlKemSeed& z,
                                           std::span<const std::uint8_t> ciphertext) {
  const auto input = ml_kem_concat(z, ciphertext);
  const auto expanded = shake256(input, kMlKemSharedSecretBytes);
  SharedSecret output{};
  std::copy(expanded.begin(), expanded.end(), output.bytes.begin());
  return output;
}

#ifdef PQCORE_ENABLE_TEST_SAMPLING
[[nodiscard]] inline MlKemSeed ml_kem_hash_h_test(std::span<const std::uint8_t> input) {
  const auto expanded = deterministic_test_expand(input, "ML-KEM-H-TEST", 0, kMlKemSeedBytes);
  MlKemSeed output{};
  std::copy(expanded.begin(), expanded.end(), output.begin());
  return output;
}

[[nodiscard]] inline MlKemGOutput ml_kem_g_test(std::span<const std::uint8_t> input) {
  const auto expanded =
      deterministic_test_expand(input, "ML-KEM-G-TEST", 0, 2 * kMlKemSeedBytes);
  MlKemGOutput output{};
  std::copy_n(expanded.begin(), kMlKemSharedSecretBytes, output.shared_secret.bytes.begin());
  std::copy_n(expanded.begin() + kMlKemSharedSecretBytes, kMlKemSeedBytes, output.coins.begin());
  return output;
}

[[nodiscard]] inline SharedSecret ml_kem_j_test(const MlKemSeed& z,
                                                std::span<const std::uint8_t> ciphertext) {
  const auto input = ml_kem_concat(z, ciphertext);
  const auto expanded =
      deterministic_test_expand(input, "ML-KEM-J-TEST", 0, kMlKemSharedSecretBytes);
  SharedSecret output{};
  std::copy(expanded.begin(), expanded.end(), output.bytes.begin());
  return output;
}

[[nodiscard]] inline MlKemKeyPair ml_kem_keygen_internal_512_test(
    const MlKemSeed& keygen_seed,
    const MlKemSeed& implicit_rejection_seed) {
  const auto pke_keypair = ml_kem_pke_keygen_512_test(keygen_seed);
  const auto public_key_hash = ml_kem_hash_h_test(pke_keypair.encryption_key);

  std::vector<std::uint8_t> decapsulation_key;
  decapsulation_key.reserve(kMlKem512DecapsulationKeyBytes);
  decapsulation_key.insert(decapsulation_key.end(),
                           pke_keypair.decryption_key.begin(),
                           pke_keypair.decryption_key.end());
  decapsulation_key.insert(decapsulation_key.end(),
                           pke_keypair.encryption_key.begin(),
                           pke_keypair.encryption_key.end());
  decapsulation_key.insert(decapsulation_key.end(), public_key_hash.begin(), public_key_hash.end());
  decapsulation_key.insert(decapsulation_key.end(),
                           implicit_rejection_seed.begin(),
                           implicit_rejection_seed.end());

  return {pke_keypair.encryption_key, decapsulation_key};
}

[[nodiscard]] inline MlKemInternalEncapsulation ml_kem_encaps_internal_512_test(
    const std::vector<std::uint8_t>& encapsulation_key,
    const MlKemSeed& message_seed) {
  if (encapsulation_key.size() != kMlKem512PublicKeyBytes) {
    throw std::invalid_argument{"ML-KEM-512 encapsulation key has the wrong length"};
  }

  const auto public_key_hash = ml_kem_hash_h_test(encapsulation_key);
  const auto g_input = ml_kem_concat(message_seed, public_key_hash);
  const auto g_output = ml_kem_g_test(g_input);
  const std::vector<std::uint8_t> message(message_seed.begin(), message_seed.end());
  const auto ciphertext =
      ml_kem_pke_encrypt_512_test(encapsulation_key, message, g_output.coins);
  return {MlKemCiphertext{ciphertext.bytes}, g_output.shared_secret};
}

[[nodiscard]] inline SharedSecret ml_kem_decaps_internal_512_test(
    const std::vector<std::uint8_t>& decapsulation_key,
    const std::vector<std::uint8_t>& ciphertext) {
  if (decapsulation_key.size() != kMlKem512DecapsulationKeyBytes) {
    throw std::invalid_argument{"ML-KEM-512 decapsulation key has the wrong length"};
  }
  if (ciphertext.size() != kMlKem512CiphertextBytes) {
    throw std::invalid_argument{"ML-KEM-512 ciphertext has the wrong length"};
  }

  const auto pke_decryption_key = byte_slice(decapsulation_key, 0, kMlKem512SecretKeyBytes);
  const auto pke_encryption_key =
      byte_slice(decapsulation_key, kMlKem512SecretKeyBytes, kMlKem512PublicKeyBytes);
  const auto public_key_hash = byte_slice(decapsulation_key,
                                         kMlKem512SecretKeyBytes + kMlKem512PublicKeyBytes,
                                         kMlKemSeedBytes);
  const auto implicit_rejection_bytes = byte_slice(
      decapsulation_key,
      kMlKem512SecretKeyBytes + kMlKem512PublicKeyBytes + kMlKemSeedBytes,
      kMlKemSeedBytes);
  MlKemSeed implicit_rejection_seed{};
  std::copy(implicit_rejection_bytes.begin(),
            implicit_rejection_bytes.end(),
            implicit_rejection_seed.begin());

  const auto recovered_message = ml_kem_pke_decrypt_512(pke_decryption_key, ciphertext);
  const auto g_input = ml_kem_concat(recovered_message, public_key_hash);
  const auto g_output = ml_kem_g_test(g_input);
  const auto fallback_secret = ml_kem_j_test(implicit_rejection_seed, ciphertext);
  const auto reencryption =
      ml_kem_pke_encrypt_512_test(pke_encryption_key, recovered_message, g_output.coins);
  const auto valid_ciphertext_mask = ml_kem_constant_time_equal_mask(ciphertext, reencryption.bytes);

  return ml_kem_select_shared_secret(
      g_output.shared_secret, fallback_secret, valid_ciphertext_mask);
}
#else
[[nodiscard]] inline MlKemSeed ml_kem_hash_h_test(std::span<const std::uint8_t> input) {
  (void)input;
  throw std::logic_error{"ML-KEM H test hook requires PQCORE_ENABLE_TEST_SAMPLING"};
}

[[nodiscard]] inline MlKemGOutput ml_kem_g_test(std::span<const std::uint8_t> input) {
  (void)input;
  throw std::logic_error{"ML-KEM G test hook requires PQCORE_ENABLE_TEST_SAMPLING"};
}

[[nodiscard]] inline SharedSecret ml_kem_j_test(const MlKemSeed& z,
                                                std::span<const std::uint8_t> ciphertext) {
  (void)z;
  (void)ciphertext;
  throw std::logic_error{"ML-KEM J test hook requires PQCORE_ENABLE_TEST_SAMPLING"};
}

[[nodiscard]] inline MlKemKeyPair ml_kem_keygen_internal_512_test(
    const MlKemSeed& keygen_seed,
    const MlKemSeed& implicit_rejection_seed) {
  (void)keygen_seed;
  (void)implicit_rejection_seed;
  throw std::logic_error{
      "ML-KEM-512 internal test keygen requires PQCORE_ENABLE_TEST_SAMPLING"};
}

[[nodiscard]] inline MlKemInternalEncapsulation ml_kem_encaps_internal_512_test(
    const std::vector<std::uint8_t>& encapsulation_key,
    const MlKemSeed& message_seed) {
  (void)encapsulation_key;
  (void)message_seed;
  throw std::logic_error{
      "ML-KEM-512 internal test encapsulation requires PQCORE_ENABLE_TEST_SAMPLING"};
}

[[nodiscard]] inline SharedSecret ml_kem_decaps_internal_512_test(
    const std::vector<std::uint8_t>& decapsulation_key,
    const std::vector<std::uint8_t>& ciphertext) {
  (void)decapsulation_key;
  (void)ciphertext;
  throw std::logic_error{
      "ML-KEM-512 internal test decapsulation requires PQCORE_ENABLE_TEST_SAMPLING"};
}
#endif

}  // namespace pqcore
