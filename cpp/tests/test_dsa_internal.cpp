#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

#include "pqcore/dsa_internal.hpp"

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

void assert_internal_flow(pqcore::MlDsaParameterSet parameter_set) {
  const auto selected = pqcore::params(parameter_set);
  pqcore::MlDsaSeed seed{};
  pqcore::MlDsaSigningRandom signing_random{};
  for (std::size_t index = 0; index < seed.size(); ++index) {
    seed[index] = static_cast<std::uint8_t>(index);
    signing_random[index] = static_cast<std::uint8_t>(0xf0u - index);
  }

  const auto keypair = pqcore::ml_dsa_keygen_internal_test(parameter_set, seed);
  assert(keypair.public_key.size() == selected.public_key_bytes);
  assert(keypair.secret_key.size() == selected.secret_key_bytes);
  assert(first_bytes(keypair.public_key, pqcore::kMlDsaSeedBytes) ==
         first_bytes(keypair.secret_key, pqcore::kMlDsaSeedBytes));
  const auto public_key_hash = pqcore::ml_dsa_hash_tr_test(keypair.public_key);
  assert(std::vector<std::uint8_t>(
             keypair.secret_key.begin() + static_cast<std::ptrdiff_t>(2 * pqcore::kMlDsaSeedBytes),
             keypair.secret_key.begin() + static_cast<std::ptrdiff_t>(
                 (2 * pqcore::kMlDsaSeedBytes) + pqcore::kMlDsaPublicKeyHashBytes)) ==
         public_key_hash);

  const std::vector<std::uint8_t> message{'m', 's', 'g'};
  const std::vector<std::uint8_t> context{'c', 't', 'x'};
  const auto formatted = pqcore::ml_dsa_format_message(message, context);
  assert(formatted[0] == 0);
  assert(formatted[1] == context.size());
  assert(std::vector<std::uint8_t>(formatted.begin() + 2, formatted.begin() + 5) == context);
  assert(std::vector<std::uint8_t>(formatted.begin() + 5, formatted.end()) == message);

  const auto challenge_bytes = pqcore::ml_dsa_signature_challenge_bytes(selected);
  const auto response_bytes = pqcore::ml_dsa_signature_response_bytes(selected);
  const auto hint_bytes = pqcore::ml_dsa_signature_hint_bytes(selected);
  assert(challenge_bytes + response_bytes + hint_bytes == selected.signature_bytes);

  const auto signing =
      pqcore::ml_dsa_sign_internal_test(parameter_set, keypair.secret_key, formatted, signing_random);
  assert(signing.signature.bytes.size() == selected.signature_bytes);
  assert(signing.rejection_iterations == 1);
  assert(pqcore::ml_dsa_verify_internal_test(
      parameter_set, keypair.public_key, formatted, signing.signature.bytes));

  const auto hints = std::span<const std::uint8_t>(
      signing.signature.bytes.data() + static_cast<std::ptrdiff_t>(challenge_bytes + response_bytes),
      hint_bytes);
  assert(pqcore::ml_dsa_hint_weight_test(hints) == selected.k);

  auto modified_message = formatted;
  modified_message.back() ^= 0x01u;
  assert(!pqcore::ml_dsa_verify_internal_test(
      parameter_set, keypair.public_key, modified_message, signing.signature.bytes));

  auto modified_public_key = keypair.public_key;
  modified_public_key[0] ^= 0x01u;
  assert(!pqcore::ml_dsa_verify_internal_test(
      parameter_set, modified_public_key, formatted, signing.signature.bytes));

  auto modified_signature = signing.signature.bytes;
  modified_signature[0] ^= 0x01u;
  assert(!pqcore::ml_dsa_verify_internal_test(
      parameter_set, keypair.public_key, formatted, modified_signature));

  auto modified_response = signing.signature.bytes;
  modified_response[challenge_bytes] = 0xff;
  assert(!pqcore::ml_dsa_verify_internal_test(
      parameter_set, keypair.public_key, formatted, modified_response));

  auto too_many_hints = signing.signature.bytes;
  const auto hint_offset = challenge_bytes + response_bytes;
  for (std::size_t index = 0; index <= selected.omega && index < hint_bytes; ++index) {
    too_many_hints[hint_offset + index] = 1;
  }
  assert(!pqcore::ml_dsa_verify_internal_test(
      parameter_set, keypair.public_key, formatted, too_many_hints));

  assert(!pqcore::ml_dsa_verify_internal_test(
      parameter_set,
      std::vector<std::uint8_t>(selected.public_key_bytes - 1),
      formatted,
      signing.signature.bytes));
  assert(!pqcore::ml_dsa_verify_internal_test(
      parameter_set,
      keypair.public_key,
      formatted,
      std::vector<std::uint8_t>(selected.signature_bytes - 1)));
  assert_throws_invalid_argument([&] {
    (void)pqcore::ml_dsa_sign_internal_test(
        parameter_set, std::vector<std::uint8_t>(selected.secret_key_bytes - 1), formatted,
        signing_random);
  });
}

int main() {
  static_assert(pqcore::kDeterministicTestSamplingEnabled);
  static_assert(pqcore::ml_dsa_signature_challenge_bytes(
                    pqcore::params(pqcore::MlDsaParameterSet::MlDsa44)) == 32);
  static_assert(pqcore::ml_dsa_signature_response_bytes(
                    pqcore::params(pqcore::MlDsaParameterSet::MlDsa44)) == 2304);
  static_assert(pqcore::ml_dsa_signature_hint_bytes(
                    pqcore::params(pqcore::MlDsaParameterSet::MlDsa44)) == 84);

  assert_internal_flow(pqcore::MlDsaParameterSet::MlDsa44);
  assert_internal_flow(pqcore::MlDsaParameterSet::MlDsa65);
  assert_internal_flow(pqcore::MlDsaParameterSet::MlDsa87);

  assert_throws_invalid_argument([] {
    const std::vector<std::uint8_t> message{'m'};
    const std::vector<std::uint8_t> context(256, 0);
    (void)pqcore::ml_dsa_format_message(message, context);
  });

  pqcore::MlDsaSeed seed{};
  for (std::size_t index = 0; index < seed.size(); ++index) {
    seed[index] = static_cast<std::uint8_t>(index);
  }
  const auto keypair =
      pqcore::ml_dsa_keygen_internal_test(pqcore::MlDsaParameterSet::MlDsa44, seed);
  pqcore::MlDsaSigningRandom signing_random{};
  for (std::size_t index = 0; index < signing_random.size(); ++index) {
    signing_random[index] = static_cast<std::uint8_t>(0xf0u - index);
  }
  const std::vector<std::uint8_t> message{'m', 's', 'g'};
  const std::vector<std::uint8_t> context{'c', 't', 'x'};
  const auto formatted = pqcore::ml_dsa_format_message(message, context);
  const auto signing = pqcore::ml_dsa_sign_internal_test(
      pqcore::MlDsaParameterSet::MlDsa44, keypair.secret_key, formatted, signing_random);
  const std::vector<std::uint8_t> expected_public_key_prefix{
      207, 120, 223, 146, 118, 126, 60, 57, 13, 255, 93, 1, 36, 124, 230, 63};
  const std::vector<std::uint8_t> expected_secret_key_prefix{
      207, 120, 223, 146, 118, 126, 60, 57, 13, 255, 93, 1, 36, 124, 230, 63};
  const std::vector<std::uint8_t> expected_signature_prefix{
      18, 118, 112, 144, 175, 96, 46, 119, 71, 35, 213, 146, 159, 67, 71, 136};
  assert(first_bytes(keypair.public_key, 16) == expected_public_key_prefix);
  assert(first_bytes(keypair.secret_key, 16) == expected_secret_key_prefix);
  assert(first_bytes(signing.signature.bytes, 16) == expected_signature_prefix);
}
