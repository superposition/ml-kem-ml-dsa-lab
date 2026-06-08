#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "pqcore/dsa_internal.hpp"
#include "pqcore/kem_internal.hpp"
#include "pqcore/pke.hpp"
#include "pqcore/sampling.hpp"
#include "pqcore/sha3.hpp"

template <std::size_t N>
std::array<std::uint8_t, N> counting_bytes(std::uint8_t first) {
  std::array<std::uint8_t, N> bytes{};
  for (std::size_t index = 0; index < bytes.size(); ++index) {
    bytes[index] = static_cast<std::uint8_t>(first + index);
  }
  return bytes;
}

std::vector<std::uint8_t> concat(std::span<const std::uint8_t> lhs,
                                 std::span<const std::uint8_t> rhs) {
  std::vector<std::uint8_t> output;
  output.reserve(lhs.size() + rhs.size());
  output.insert(output.end(), lhs.begin(), lhs.end());
  output.insert(output.end(), rhs.begin(), rhs.end());
  return output;
}

int main() {
  const auto kem_seed = counting_bytes<pqcore::kMlKemSeedBytes>(3);
  const std::uint8_t nonce = 9;

  std::array<std::uint8_t, pqcore::kMlKemSeedBytes + 1> prf_input{};
  std::copy(kem_seed.begin(), kem_seed.end(), prf_input.begin());
  prf_input[pqcore::kMlKemSeedBytes] = nonce;
  assert(pqcore::ml_kem_prf(kem_seed, nonce, 2) == pqcore::shake256(prf_input, 128));
  assert(pqcore::ml_kem_prf(kem_seed, nonce, 3) == pqcore::shake256(prf_input, 192));

  pqcore::MlKemSampleNttSeed sample_ntt_seed{};
  std::copy(kem_seed.begin(), kem_seed.end(), sample_ntt_seed.begin());
  sample_ntt_seed[32] = 1;
  sample_ntt_seed[33] = 2;
  assert(pqcore::ml_kem_xof(sample_ntt_seed, 64) == pqcore::shake128(sample_ntt_seed, 64));
  assert(pqcore::ml_kem_sample_ntt(sample_ntt_seed).coeff_at(0).value() ==
         pqcore::ml_kem_sample_ntt_from_bytes(pqcore::shake128(sample_ntt_seed, 3 * 1024))
             .coeff_at(0)
             .value());

  const auto expanded = pqcore::ml_kem_pke_expand_keygen_seeds_512(kem_seed);
  const auto expanded_digest = pqcore::sha3_512(kem_seed);
  assert(std::equal(expanded_digest.begin(),
                    expanded_digest.begin() + pqcore::kMlKemSeedBytes,
                    expanded[0].begin()));
  assert(std::equal(expanded_digest.begin() + pqcore::kMlKemSeedBytes,
                    expanded_digest.end(),
                    expanded[1].begin()));
  const auto matrix_seed = pqcore::ml_kem_pke_matrix_seed(expanded[0], 1, 0);
  assert(matrix_seed[32] == 1);
  assert(matrix_seed[33] == 0);
  const auto noise = pqcore::ml_kem_pke_sample_noise_poly_512(expanded[1], 3, 0);
  assert(noise.coeff_at(0).value() < pqcore::MlKemField::modulus());

  const std::vector<std::uint8_t> public_bytes{'p', 'u', 'b'};
  const auto h = pqcore::ml_kem_hash_h(public_bytes);
  const auto h_digest = pqcore::sha3_256(public_bytes);
  assert(std::equal(h.begin(), h.end(), h_digest.begin()));

  const auto g = pqcore::ml_kem_g(public_bytes);
  const auto g_digest = pqcore::sha3_512(public_bytes);
  assert(std::equal(g.shared_secret.bytes.begin(),
                    g.shared_secret.bytes.end(),
                    g_digest.begin()));
  assert(std::equal(g.coins.begin(),
                    g.coins.end(),
                    g_digest.begin() + pqcore::kMlKemSharedSecretBytes));
  const auto j = pqcore::ml_kem_j(kem_seed, public_bytes);
  const auto j_expected = pqcore::shake256(concat(kem_seed, public_bytes), j.bytes.size());
  assert(std::equal(j.bytes.begin(), j.bytes.end(), j_expected.begin()));

  const auto dsa_signing_key = counting_bytes<pqcore::kMlDsaSeedBytes>(20);
  const auto dsa_random = counting_bytes<pqcore::kMlDsaSigningRandomBytes>(80);
  const std::vector<std::uint8_t> message{'m', 's', 'g'};
  const std::vector<std::uint8_t> context{'c', 't', 'x'};
  const auto formatted = pqcore::ml_dsa_format_message(message, context);
  const std::vector<std::uint8_t> public_key(1312, 7);
  const auto tr = pqcore::ml_dsa_hash_tr(public_key);
  assert(tr == pqcore::shake256(public_key, pqcore::kMlDsaPublicKeyHashBytes));

  const auto mu = pqcore::ml_dsa_message_representative(tr, formatted);
  assert(mu == pqcore::shake256(concat(tr, formatted), pqcore::kMlDsaPublicKeyHashBytes));

  const auto rho_second = pqcore::ml_dsa_signing_seed(dsa_signing_key, dsa_random, mu);
  const auto signing_prefix = concat(dsa_signing_key, dsa_random);
  assert(rho_second ==
         pqcore::shake256(concat(signing_prefix, mu), pqcore::kMlDsaPublicKeyHashBytes));

  const std::vector<std::uint8_t> witness(64, 5);
  const auto challenge =
      pqcore::ml_dsa_commitment_hash(pqcore::MlDsaParameterSet::MlDsa44, mu, witness);
  assert(challenge.size() ==
         pqcore::ml_dsa_signature_challenge_bytes(
             pqcore::params(pqcore::MlDsaParameterSet::MlDsa44)));
  assert(challenge == pqcore::shake256(concat(mu, witness), challenge.size()));
}
