#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "pqcore/pke.hpp"

int main() {
  static_assert(!pqcore::kDeterministicTestSamplingEnabled);

  pqcore::MlKemSeed seed{};
  bool threw = false;
  try {
    (void)pqcore::ml_kem_pke_keygen_512_test(seed);
  } catch (const std::logic_error&) {
    threw = true;
  }
  assert(threw);

  threw = false;
  try {
    (void)pqcore::ml_kem_pke_encrypt_512_test(std::vector<std::uint8_t>(800),
                                              std::vector<std::uint8_t>(32),
                                              seed);
  } catch (const std::logic_error&) {
    threw = true;
  }
  assert(threw);

  threw = false;
  try {
    (void)pqcore::ml_kem_pke_decrypt_512(std::vector<std::uint8_t>(768),
                                         std::vector<std::uint8_t>(768));
  } catch (const std::logic_error&) {
    threw = true;
  }
  assert(threw);
}
