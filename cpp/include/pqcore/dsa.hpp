#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "pqcore/kem.hpp"
#include "pqcore/params.hpp"

namespace pqcore {

struct MlDsaKeyPair {
  std::vector<std::uint8_t> public_key;
  std::vector<std::uint8_t> secret_key;
};

struct Signature {
  std::vector<std::uint8_t> bytes;
};

[[nodiscard]] inline MlDsaKeyPair ml_dsa_keygen(MlDsaParameterSet parameter_set) {
  const auto selected = params(parameter_set);
  (void)selected;
  throw NotImplemented{"ML-DSA key generation requires FIPS 204 algorithm and vector coverage"};
}

[[nodiscard]] inline Signature ml_dsa_sign(
    MlDsaParameterSet parameter_set,
    const std::vector<std::uint8_t>& secret_key,
    const std::vector<std::uint8_t>& message,
    const std::vector<std::uint8_t>& context) {
  const auto selected = params(parameter_set);
  if (secret_key.size() != selected.secret_key_bytes) {
    throw std::invalid_argument{"secret key has the wrong length"};
  }
  if (context.size() > 255) {
    throw std::invalid_argument{"context is longer than 255 bytes"};
  }
  (void)message;
  throw NotImplemented{"ML-DSA signing requires FIPS 204 algorithm and vector coverage"};
}

[[nodiscard]] inline bool ml_dsa_verify(
    MlDsaParameterSet parameter_set,
    const std::vector<std::uint8_t>& public_key,
    const std::vector<std::uint8_t>& message,
    const std::vector<std::uint8_t>& signature,
    const std::vector<std::uint8_t>& context) {
  const auto selected = params(parameter_set);
  if (public_key.size() != selected.public_key_bytes) {
    throw std::invalid_argument{"public key has the wrong length"};
  }
  if (signature.size() != selected.signature_bytes) {
    throw std::invalid_argument{"signature has the wrong length"};
  }
  if (context.size() > 255) {
    throw std::invalid_argument{"context is longer than 255 bytes"};
  }
  (void)message;
  throw NotImplemented{"ML-DSA verification requires FIPS 204 algorithm and vector coverage"};
}

}  // namespace pqcore

