#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

#include "pqcore/dsa.hpp"
#include "pqcore/params.hpp"
#include "pqcore/sampling.hpp"

namespace pqcore {

inline constexpr std::size_t kMlDsaSeedBytes = 32;
inline constexpr std::size_t kMlDsaSigningRandomBytes = 32;
inline constexpr std::size_t kMlDsaPublicKeyHashBytes = 64;

using MlDsaSeed = std::array<std::uint8_t, kMlDsaSeedBytes>;
using MlDsaSigningRandom = std::array<std::uint8_t, kMlDsaSigningRandomBytes>;

struct MlDsaInternalSignResult {
  Signature signature;
  std::size_t rejection_iterations;
};

[[nodiscard]] constexpr std::uint16_t ml_dsa_parameter_nonce(
    MlDsaParameterSet parameter_set) {
  switch (parameter_set) {
    case MlDsaParameterSet::MlDsa44:
      return 44;
    case MlDsaParameterSet::MlDsa65:
      return 65;
    case MlDsaParameterSet::MlDsa87:
      return 87;
  }
  return 0;
}

[[nodiscard]] constexpr std::size_t ml_dsa_bit_length(std::uint32_t value) {
  std::size_t bits = 0;
  do {
    ++bits;
    value >>= 1u;
  } while (value != 0);
  return bits;
}

[[nodiscard]] constexpr std::size_t ml_dsa_signature_challenge_bytes(
    const MlDsaParams& selected) {
  return selected.lambda_bits / 4;
}

[[nodiscard]] constexpr std::size_t ml_dsa_signature_response_bytes(
    const MlDsaParams& selected) {
  return selected.l * 32 * (1 + ml_dsa_bit_length(selected.gamma1 - 1));
}

[[nodiscard]] constexpr std::size_t ml_dsa_signature_hint_bytes(
    const MlDsaParams& selected) {
  return selected.omega + selected.k;
}

[[nodiscard]] constexpr std::size_t ml_dsa_signature_witness_bytes(
    const MlDsaParams& selected) {
  return ml_dsa_signature_response_bytes(selected) + ml_dsa_signature_hint_bytes(selected);
}

[[nodiscard]] constexpr std::size_t ml_dsa_secret_key_header_bytes() {
  return kMlDsaSeedBytes + kMlDsaSeedBytes + kMlDsaPublicKeyHashBytes;
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_dsa_concat(
    std::span<const std::uint8_t> lhs,
    std::span<const std::uint8_t> rhs) {
  std::vector<std::uint8_t> output;
  output.reserve(lhs.size() + rhs.size());
  output.insert(output.end(), lhs.begin(), lhs.end());
  output.insert(output.end(), rhs.begin(), rhs.end());
  return output;
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_dsa_byte_slice(
    std::span<const std::uint8_t> bytes,
    std::size_t offset,
    std::size_t length) {
  if (offset > bytes.size() || length > bytes.size() - offset) {
    throw std::invalid_argument{"ML-DSA byte slice is out of range"};
  }
  return std::vector<std::uint8_t>(bytes.begin() + static_cast<std::ptrdiff_t>(offset),
                                   bytes.begin() + static_cast<std::ptrdiff_t>(offset + length));
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_dsa_format_message(
    std::span<const std::uint8_t> message,
    std::span<const std::uint8_t> context) {
  if (context.size() > 255) {
    throw std::invalid_argument{"context is longer than 255 bytes"};
  }

  std::vector<std::uint8_t> formatted;
  formatted.reserve(2 + context.size() + message.size());
  formatted.push_back(0);
  formatted.push_back(static_cast<std::uint8_t>(context.size()));
  formatted.insert(formatted.end(), context.begin(), context.end());
  formatted.insert(formatted.end(), message.begin(), message.end());
  return formatted;
}

[[nodiscard]] inline std::size_t ml_dsa_hint_weight_test(
    std::span<const std::uint8_t> hint_bytes) {
  std::size_t weight = 0;
  for (const auto value : hint_bytes) {
    if (value != 0) {
      ++weight;
    }
  }
  return weight;
}

[[nodiscard]] inline bool ml_dsa_response_within_bound_test(
    std::span<const std::uint8_t> response_bytes) {
  return std::all_of(response_bytes.begin(), response_bytes.end(), [](std::uint8_t value) {
    return value < 251;
  });
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_dsa_hash(
    std::span<const std::uint8_t> input,
    std::size_t output_bytes) {
  return shake256(input, output_bytes);
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_dsa_hash_tr(
    std::span<const std::uint8_t> public_key) {
  return ml_dsa_hash(public_key, kMlDsaPublicKeyHashBytes);
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_dsa_message_representative(
    std::span<const std::uint8_t> public_key_hash,
    std::span<const std::uint8_t> formatted_message) {
  if (public_key_hash.size() != kMlDsaPublicKeyHashBytes) {
    throw std::invalid_argument{"ML-DSA public-key hash has the wrong length"};
  }
  return ml_dsa_hash(ml_dsa_concat(public_key_hash, formatted_message), kMlDsaPublicKeyHashBytes);
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_dsa_signing_seed(
    std::span<const std::uint8_t> signing_key,
    const MlDsaSigningRandom& signing_random,
    std::span<const std::uint8_t> message_representative) {
  if (signing_key.size() != kMlDsaSeedBytes) {
    throw std::invalid_argument{"ML-DSA signing key seed has the wrong length"};
  }
  if (message_representative.size() != kMlDsaPublicKeyHashBytes) {
    throw std::invalid_argument{"ML-DSA message representative has the wrong length"};
  }
  const auto signing_seed_prefix = ml_dsa_concat(signing_key, signing_random);
  return ml_dsa_hash(
      ml_dsa_concat(signing_seed_prefix, message_representative), kMlDsaPublicKeyHashBytes);
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_dsa_commitment_hash(
    MlDsaParameterSet parameter_set,
    std::span<const std::uint8_t> message_representative,
    std::span<const std::uint8_t> witness) {
  if (message_representative.size() != kMlDsaPublicKeyHashBytes) {
    throw std::invalid_argument{"ML-DSA message representative has the wrong length"};
  }
  return ml_dsa_hash(
      ml_dsa_concat(message_representative, witness),
      ml_dsa_signature_challenge_bytes(params(parameter_set)));
}

#ifdef PQCORE_ENABLE_TEST_SAMPLING
[[nodiscard]] inline std::vector<std::uint8_t> ml_dsa_hash_tr_test(
    std::span<const std::uint8_t> public_key) {
  return deterministic_test_expand(
      public_key, "ML-DSA-TR-TEST", 0, kMlDsaPublicKeyHashBytes);
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_dsa_commitment_hash_test(
    MlDsaParameterSet parameter_set,
    std::span<const std::uint8_t> public_key_hash,
    std::span<const std::uint8_t> formatted_message,
    std::span<const std::uint8_t> witness) {
  const auto message_representative_input = ml_dsa_concat(public_key_hash, formatted_message);
  const auto message_representative = deterministic_test_expand(
      message_representative_input,
      "ML-DSA-MU-TEST",
      ml_dsa_parameter_nonce(parameter_set),
      kMlDsaPublicKeyHashBytes);
  const auto commitment_input = ml_dsa_concat(message_representative, witness);
  return deterministic_test_expand(
      commitment_input,
      "ML-DSA-CTILDE-TEST",
      ml_dsa_parameter_nonce(parameter_set),
      ml_dsa_signature_challenge_bytes(params(parameter_set)));
}

[[nodiscard]] inline MlDsaKeyPair ml_dsa_keygen_internal_test(
    MlDsaParameterSet parameter_set,
    const MlDsaSeed& seed) {
  const auto selected = params(parameter_set);
  const auto keygen_input = ml_dsa_concat(
      seed,
      std::array<std::uint8_t, 2>{
          static_cast<std::uint8_t>(selected.k),
          static_cast<std::uint8_t>(selected.l)});
  const auto expanded = deterministic_test_expand(
      keygen_input, "ML-DSA-KEYGEN-TEST", ml_dsa_parameter_nonce(parameter_set), 128);
  const auto rho = ml_dsa_byte_slice(expanded, 0, kMlDsaSeedBytes);
  const auto signing_key = ml_dsa_byte_slice(expanded, 96, kMlDsaSeedBytes);

  auto public_key = deterministic_test_expand(
      keygen_input,
      "ML-DSA-PK-BODY-TEST",
      ml_dsa_parameter_nonce(parameter_set),
      selected.public_key_bytes);
  std::copy(rho.begin(), rho.end(), public_key.begin());
  const auto public_key_hash = ml_dsa_hash_tr_test(public_key);

  auto secret_key = deterministic_test_expand(
      keygen_input,
      "ML-DSA-SK-BODY-TEST",
      ml_dsa_parameter_nonce(parameter_set),
      selected.secret_key_bytes);
  std::copy(rho.begin(), rho.end(), secret_key.begin());
  std::copy(signing_key.begin(), signing_key.end(), secret_key.begin() + kMlDsaSeedBytes);
  std::copy(public_key_hash.begin(),
            public_key_hash.end(),
            secret_key.begin() + (2 * kMlDsaSeedBytes));

  return {public_key, secret_key};
}

[[nodiscard]] inline MlDsaInternalSignResult ml_dsa_sign_internal_test(
    MlDsaParameterSet parameter_set,
    const std::vector<std::uint8_t>& secret_key,
    const std::vector<std::uint8_t>& formatted_message,
    const MlDsaSigningRandom& signing_random) {
  const auto selected = params(parameter_set);
  if (secret_key.size() != selected.secret_key_bytes) {
    throw std::invalid_argument{"ML-DSA internal secret key has the wrong length"};
  }

  const auto signing_key =
      ml_dsa_byte_slice(secret_key, kMlDsaSeedBytes, kMlDsaSeedBytes);
  const auto public_key_hash =
      ml_dsa_byte_slice(secret_key, 2 * kMlDsaSeedBytes, kMlDsaPublicKeyHashBytes);
  const auto message_representative_input = ml_dsa_concat(public_key_hash, formatted_message);
  const auto message_representative = deterministic_test_expand(
      message_representative_input,
      "ML-DSA-MU-TEST",
      ml_dsa_parameter_nonce(parameter_set),
      kMlDsaPublicKeyHashBytes);
  const auto signing_seed_input = ml_dsa_concat(
      ml_dsa_concat(signing_key, signing_random), message_representative);
  const auto signing_seed = deterministic_test_expand(
      signing_seed_input,
      "ML-DSA-RHO-SECOND-TEST",
      ml_dsa_parameter_nonce(parameter_set),
      kMlDsaPublicKeyHashBytes);

  const auto challenge_bytes = ml_dsa_signature_challenge_bytes(selected);
  const auto response_bytes = ml_dsa_signature_response_bytes(selected);
  const auto hint_bytes = ml_dsa_signature_hint_bytes(selected);
  const auto witness_bytes = response_bytes + hint_bytes;

  std::size_t rejected = 0;
  std::vector<std::uint8_t> witness;
  for (std::size_t attempt = 0; attempt < 8; ++attempt) {
    witness = deterministic_test_expand(
        signing_seed,
        "ML-DSA-WITNESS-TEST",
        static_cast<std::uint16_t>(attempt * selected.l),
        witness_bytes);
    if (attempt == 0) {
      ++rejected;
      continue;
    }

    for (std::size_t index = 0; index < response_bytes; ++index) {
      witness[index] = static_cast<std::uint8_t>(witness[index] % 251);
    }
    std::fill(witness.begin() + static_cast<std::ptrdiff_t>(response_bytes),
              witness.end(),
              0);
    for (std::size_t index = 0; index < selected.k && index < selected.omega; ++index) {
      witness[response_bytes + index] = 1;
    }
    break;
  }

  const auto challenge = ml_dsa_commitment_hash_test(
      parameter_set, public_key_hash, formatted_message, witness);
  std::vector<std::uint8_t> signature;
  signature.reserve(challenge_bytes + witness.size());
  signature.insert(signature.end(), challenge.begin(), challenge.end());
  signature.insert(signature.end(), witness.begin(), witness.end());
  return {Signature{signature}, rejected};
}

[[nodiscard]] inline bool ml_dsa_verify_internal_test(
    MlDsaParameterSet parameter_set,
    const std::vector<std::uint8_t>& public_key,
    const std::vector<std::uint8_t>& formatted_message,
    const std::vector<std::uint8_t>& signature) {
  const auto selected = params(parameter_set);
  if (public_key.size() != selected.public_key_bytes ||
      signature.size() != selected.signature_bytes) {
    return false;
  }

  const auto challenge_bytes = ml_dsa_signature_challenge_bytes(selected);
  const auto response_bytes = ml_dsa_signature_response_bytes(selected);
  const auto hint_bytes = ml_dsa_signature_hint_bytes(selected);
  const auto stored_challenge = ml_dsa_byte_slice(signature, 0, challenge_bytes);
  const auto witness = ml_dsa_byte_slice(signature, challenge_bytes, response_bytes + hint_bytes);
  const auto response = std::span<const std::uint8_t>(witness.data(), response_bytes);
  const auto hints = std::span<const std::uint8_t>(
      witness.data() + static_cast<std::ptrdiff_t>(response_bytes), hint_bytes);
  if (!ml_dsa_response_within_bound_test(response) ||
      ml_dsa_hint_weight_test(hints) > selected.omega) {
    return false;
  }

  const auto public_key_hash = ml_dsa_hash_tr_test(public_key);
  const auto expected_challenge = ml_dsa_commitment_hash_test(
      parameter_set, public_key_hash, formatted_message, witness);
  return stored_challenge == expected_challenge;
}
#else
[[nodiscard]] inline std::vector<std::uint8_t> ml_dsa_hash_tr_test(
    std::span<const std::uint8_t> public_key) {
  (void)public_key;
  throw std::logic_error{"ML-DSA tr test hook requires PQCORE_ENABLE_TEST_SAMPLING"};
}

[[nodiscard]] inline MlDsaKeyPair ml_dsa_keygen_internal_test(
    MlDsaParameterSet parameter_set,
    const MlDsaSeed& seed) {
  (void)parameter_set;
  (void)seed;
  throw std::logic_error{"ML-DSA internal test keygen requires PQCORE_ENABLE_TEST_SAMPLING"};
}

[[nodiscard]] inline MlDsaInternalSignResult ml_dsa_sign_internal_test(
    MlDsaParameterSet parameter_set,
    const std::vector<std::uint8_t>& secret_key,
    const std::vector<std::uint8_t>& formatted_message,
    const MlDsaSigningRandom& signing_random) {
  (void)parameter_set;
  (void)secret_key;
  (void)formatted_message;
  (void)signing_random;
  throw std::logic_error{"ML-DSA internal test signing requires PQCORE_ENABLE_TEST_SAMPLING"};
}

[[nodiscard]] inline bool ml_dsa_verify_internal_test(
    MlDsaParameterSet parameter_set,
    const std::vector<std::uint8_t>& public_key,
    const std::vector<std::uint8_t>& formatted_message,
    const std::vector<std::uint8_t>& signature) {
  (void)parameter_set;
  (void)public_key;
  (void)formatted_message;
  (void)signature;
  throw std::logic_error{"ML-DSA internal test verification requires PQCORE_ENABLE_TEST_SAMPLING"};
}
#endif

}  // namespace pqcore
