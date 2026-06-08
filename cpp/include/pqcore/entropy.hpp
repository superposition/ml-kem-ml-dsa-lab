#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "pqcore/params.hpp"

namespace pqcore {

enum class EntropyPurpose {
  MlKemKeygenSeed,
  MlKemImplicitRejectionSeed,
  MlKemEncapsulationSeed,
  MlDsaKeygenSeed,
  MlDsaSigningRandom,
};

struct EntropyRequest {
  EntropyPurpose purpose;
  std::string_view label;
  std::size_t output_bytes;
  std::size_t required_strength_bits;
};

[[nodiscard]] constexpr std::string_view entropy_purpose_name(EntropyPurpose purpose) noexcept {
  switch (purpose) {
    case EntropyPurpose::MlKemKeygenSeed:
      return "ml-kem-keygen-seed";
    case EntropyPurpose::MlKemImplicitRejectionSeed:
      return "ml-kem-implicit-rejection-seed";
    case EntropyPurpose::MlKemEncapsulationSeed:
      return "ml-kem-encapsulation-seed";
    case EntropyPurpose::MlDsaKeygenSeed:
      return "ml-dsa-keygen-seed";
    case EntropyPurpose::MlDsaSigningRandom:
      return "ml-dsa-signing-random";
  }
  return "unknown";
}

[[nodiscard]] constexpr EntropyRequest ml_kem_entropy_request(
    MlKemParameterSet parameter_set,
    EntropyPurpose purpose) {
  const auto selected = params(parameter_set);
  switch (purpose) {
    case EntropyPurpose::MlKemKeygenSeed:
    case EntropyPurpose::MlKemImplicitRejectionSeed:
    case EntropyPurpose::MlKemEncapsulationSeed:
      return {purpose, entropy_purpose_name(purpose), 32, selected.rbg_strength_bits};
    case EntropyPurpose::MlDsaKeygenSeed:
    case EntropyPurpose::MlDsaSigningRandom:
      throw std::invalid_argument{"ML-KEM entropy request received an ML-DSA purpose"};
  }
  throw std::invalid_argument{"unknown ML-KEM entropy purpose"};
}

[[nodiscard]] constexpr EntropyRequest ml_dsa_entropy_request(
    MlDsaParameterSet parameter_set,
    EntropyPurpose purpose) {
  const auto selected = params(parameter_set);
  switch (purpose) {
    case EntropyPurpose::MlDsaKeygenSeed:
    case EntropyPurpose::MlDsaSigningRandom:
      return {purpose, entropy_purpose_name(purpose), 32, selected.lambda_bits};
    case EntropyPurpose::MlKemKeygenSeed:
    case EntropyPurpose::MlKemImplicitRejectionSeed:
    case EntropyPurpose::MlKemEncapsulationSeed:
      throw std::invalid_argument{"ML-DSA entropy request received an ML-KEM purpose"};
  }
  throw std::invalid_argument{"unknown ML-DSA entropy purpose"};
}

[[nodiscard]] inline std::vector<std::uint8_t> production_random_bytes(
    const EntropyRequest& request) {
  if (request.output_bytes == 0) {
    throw std::invalid_argument{"entropy request must ask for at least one byte"};
  }
  if (request.required_strength_bits == 0) {
    throw std::invalid_argument{"entropy request must state required strength"};
  }
  throw std::logic_error{
      "production entropy source is not configured; see docs/production-readiness.md"};
}

[[nodiscard]] inline std::vector<std::uint8_t> production_random_bytes(
    std::size_t output_bytes) {
  return production_random_bytes(
      {EntropyPurpose::MlKemKeygenSeed, "unspecified", output_bytes, 1});
}

}  // namespace pqcore
