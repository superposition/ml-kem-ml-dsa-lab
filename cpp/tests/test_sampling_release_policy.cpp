#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include "pqcore/entropy.hpp"
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

int main() {
  static_assert(!pqcore::kDeterministicTestSamplingEnabled);

  const auto kem_request = pqcore::ml_kem_entropy_request(
      pqcore::MlKemParameterSet::MlKem768,
      pqcore::EntropyPurpose::MlKemKeygenSeed);
  assert(kem_request.label == "ml-kem-keygen-seed");
  assert(kem_request.output_bytes == 32);
  assert(kem_request.required_strength_bits == 192);

  const auto dsa_request = pqcore::ml_dsa_entropy_request(
      pqcore::MlDsaParameterSet::MlDsa87,
      pqcore::EntropyPurpose::MlDsaSigningRandom);
  assert(dsa_request.label == "ml-dsa-signing-random");
  assert(dsa_request.output_bytes == 32);
  assert(dsa_request.required_strength_bits == 256);

  assert_throws_invalid_argument([] {
    (void)pqcore::ml_kem_entropy_request(
        pqcore::MlKemParameterSet::MlKem512,
        pqcore::EntropyPurpose::MlDsaSigningRandom);
  });
  assert_throws_invalid_argument([] {
    (void)pqcore::production_random_bytes(
        {pqcore::EntropyPurpose::MlKemKeygenSeed, "bad", 0, 128});
  });
  assert_throws_invalid_argument([] {
    (void)pqcore::production_random_bytes(
        {pqcore::EntropyPurpose::MlKemKeygenSeed, "bad", 32, 0});
  });

  bool threw = false;
  try {
    (void)pqcore::production_random_bytes(kem_request);
  } catch (const std::logic_error&) {
    threw = true;
  }
  assert(threw);

  threw = false;
  try {
    (void)pqcore::production_random_bytes(32);
  } catch (const std::logic_error&) {
    threw = true;
  }
  assert(threw);
}
