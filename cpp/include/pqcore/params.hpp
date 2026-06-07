#pragma once

#include <cstdint>
#include <string_view>

namespace pqcore {

enum class MlKemParameterSet {
  MlKem512,
  MlKem768,
  MlKem1024,
};

struct MlKemParams {
  std::string_view name;
  std::size_t k;
  std::int32_t q;
  std::size_t eta1;
  std::size_t eta2;
  std::size_t du;
  std::size_t dv;
  std::size_t public_key_bytes;
  std::size_t secret_key_bytes;
  std::size_t ciphertext_bytes;
  std::size_t shared_secret_bytes;
};

[[nodiscard]] constexpr MlKemParams params(MlKemParameterSet set) noexcept {
  switch (set) {
    case MlKemParameterSet::MlKem512:
      return {"ML-KEM-512", 2, 3329, 3, 2, 10, 4, 800, 1632, 768, 32};
    case MlKemParameterSet::MlKem768:
      return {"ML-KEM-768", 3, 3329, 2, 2, 10, 4, 1184, 2400, 1088, 32};
    case MlKemParameterSet::MlKem1024:
      return {"ML-KEM-1024", 4, 3329, 2, 2, 11, 5, 1568, 3168, 1568, 32};
  }
  return {"invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
}

enum class MlDsaParameterSet {
  MlDsa44,
  MlDsa65,
  MlDsa87,
};

struct MlDsaParams {
  std::string_view name;
  std::size_t k;
  std::size_t l;
  std::int32_t q;
  std::size_t d;
  std::size_t tau;
  std::size_t lambda_bits;
  std::int32_t gamma1;
  std::int32_t gamma2;
  std::size_t eta;
  std::size_t beta;
  std::size_t omega;
  std::size_t public_key_bytes;
  std::size_t secret_key_bytes;
  std::size_t signature_bytes;
};

[[nodiscard]] constexpr MlDsaParams params(MlDsaParameterSet set) noexcept {
  switch (set) {
    case MlDsaParameterSet::MlDsa44:
      return {"ML-DSA-44", 4, 4, 8380417, 13, 39, 128, 1 << 17, (8380417 - 1) / 88, 2, 78, 80, 1312, 2560, 2420};
    case MlDsaParameterSet::MlDsa65:
      return {"ML-DSA-65", 6, 5, 8380417, 13, 49, 192, 1 << 19, (8380417 - 1) / 32, 4, 196, 55, 1952, 4032, 3309};
    case MlDsaParameterSet::MlDsa87:
      return {"ML-DSA-87", 8, 7, 8380417, 13, 60, 256, 1 << 19, (8380417 - 1) / 32, 2, 120, 75, 2592, 4896, 4627};
  }
  return {"invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
}

}  // namespace pqcore

