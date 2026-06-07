#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "pqcore/dsa_internal.hpp"

int main() {
  static_assert(!pqcore::kDeterministicTestSamplingEnabled);

  pqcore::MlDsaSeed seed{};
  pqcore::MlDsaSigningRandom signing_random{};
  bool threw = false;
  try {
    (void)pqcore::ml_dsa_keygen_internal_test(pqcore::MlDsaParameterSet::MlDsa44, seed);
  } catch (const std::logic_error&) {
    threw = true;
  }
  assert(threw);

  threw = false;
  try {
    (void)pqcore::ml_dsa_sign_internal_test(
        pqcore::MlDsaParameterSet::MlDsa44,
        std::vector<std::uint8_t>(2560),
        std::vector<std::uint8_t>{'m'},
        signing_random);
  } catch (const std::logic_error&) {
    threw = true;
  }
  assert(threw);

  threw = false;
  try {
    (void)pqcore::ml_dsa_verify_internal_test(
        pqcore::MlDsaParameterSet::MlDsa44,
        std::vector<std::uint8_t>(1312),
        std::vector<std::uint8_t>{'m'},
        std::vector<std::uint8_t>(2420));
  } catch (const std::logic_error&) {
    threw = true;
  }
  assert(threw);
}
