#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include "pqcore/sampling.hpp"

int main() {
  static_assert(!pqcore::kDeterministicTestSamplingEnabled);

  bool threw = false;
  try {
    (void)pqcore::production_random_bytes(32);
  } catch (const std::logic_error&) {
    threw = true;
  }
  assert(threw);
}

