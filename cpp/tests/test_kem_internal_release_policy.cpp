#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "pqcore/kem_internal.hpp"

int main() {
  static_assert(!pqcore::kDeterministicTestSamplingEnabled);

  pqcore::MlKemSeed seed{};
  bool threw = false;
  try {
    (void)pqcore::ml_kem_keygen_internal_512_test(seed, seed);
  } catch (const std::logic_error&) {
    threw = true;
  }
  assert(threw);

  threw = false;
  try {
    (void)pqcore::ml_kem_encaps_internal_512_test(std::vector<std::uint8_t>(800), seed);
  } catch (const std::logic_error&) {
    threw = true;
  }
  assert(threw);

  threw = false;
  try {
    (void)pqcore::ml_kem_decaps_internal_512_test(std::vector<std::uint8_t>(1632),
                                                  std::vector<std::uint8_t>(768));
  } catch (const std::logic_error&) {
    threw = true;
  }
  assert(threw);
}
