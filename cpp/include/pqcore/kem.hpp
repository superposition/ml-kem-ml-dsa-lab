#pragma once

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "pqcore/params.hpp"

namespace pqcore {

class NotImplemented : public std::logic_error {
 public:
  explicit NotImplemented(const std::string& message) : std::logic_error(message) {}
};

struct MlKemKeyPair {
  std::vector<std::uint8_t> public_key;
  std::vector<std::uint8_t> secret_key;
};

struct MlKemCiphertext {
  std::vector<std::uint8_t> bytes;
};

struct SharedSecret {
  std::array<std::uint8_t, 32> bytes{};
};

[[nodiscard]] inline MlKemKeyPair ml_kem_keygen(MlKemParameterSet parameter_set) {
  const auto selected = params(parameter_set);
  (void)selected;
  throw NotImplemented{"ML-KEM key generation requires FIPS 203 algorithm and vector coverage"};
}

[[nodiscard]] inline std::pair<MlKemCiphertext, SharedSecret> ml_kem_encapsulate(
    MlKemParameterSet parameter_set,
    const std::vector<std::uint8_t>& public_key) {
  const auto selected = params(parameter_set);
  if (public_key.size() != selected.public_key_bytes) {
    throw std::invalid_argument{"public key has the wrong length"};
  }
  throw NotImplemented{"ML-KEM encapsulation requires FIPS 203 algorithm and vector coverage"};
}

[[nodiscard]] inline SharedSecret ml_kem_decapsulate(
    MlKemParameterSet parameter_set,
    const std::vector<std::uint8_t>& secret_key,
    const std::vector<std::uint8_t>& ciphertext) {
  const auto selected = params(parameter_set);
  if (secret_key.size() != selected.secret_key_bytes) {
    throw std::invalid_argument{"secret key has the wrong length"};
  }
  if (ciphertext.size() != selected.ciphertext_bytes) {
    throw std::invalid_argument{"ciphertext has the wrong length"};
  }
  throw NotImplemented{"ML-KEM decapsulation requires FIPS 203 algorithm and vector coverage"};
}

}  // namespace pqcore
