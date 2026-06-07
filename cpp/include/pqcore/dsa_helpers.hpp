#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>

#include "pqcore/params.hpp"
#include "pqcore/polynomial.hpp"
#include "pqcore/sampling.hpp"

namespace pqcore {

struct MlDsaDecomposition {
  std::int32_t high;
  std::int32_t low;
};

[[nodiscard]] constexpr std::int32_t ml_dsa_positive_mod(std::int64_t value,
                                                         std::int32_t modulus) {
  if (modulus <= 0) {
    throw std::invalid_argument{"modulus must be positive"};
  }
  auto reduced = value % modulus;
  if (reduced < 0) {
    reduced += modulus;
  }
  return static_cast<std::int32_t>(reduced);
}

[[nodiscard]] constexpr std::int32_t ml_dsa_centered_mod(std::int64_t value,
                                                         std::int32_t modulus) {
  if (modulus <= 0 || modulus % 2 != 0) {
    throw std::invalid_argument{"centered modulus must be positive and even"};
  }
  const auto reduced = ml_dsa_positive_mod(value, modulus);
  if (reduced > modulus / 2) {
    return reduced - modulus;
  }
  return reduced;
}

[[nodiscard]] constexpr std::int32_t ml_dsa_power2round_base(
    const MlDsaParams& selected) {
  if (selected.d >= 30) {
    throw std::invalid_argument{"ML-DSA d is too large"};
  }
  return static_cast<std::int32_t>(1u << selected.d);
}

[[nodiscard]] constexpr std::int32_t ml_dsa_decompose_base(
    const MlDsaParams& selected) {
  return 2 * selected.gamma2;
}

[[nodiscard]] constexpr std::int32_t ml_dsa_high_bits_modulus(
    const MlDsaParams& selected) {
  const auto alpha = ml_dsa_decompose_base(selected);
  if ((selected.q - 1) % alpha != 0) {
    throw std::invalid_argument{"2 * gamma2 must divide q - 1"};
  }
  return (selected.q - 1) / alpha;
}

[[nodiscard]] constexpr std::size_t ml_dsa_challenge_seed_bytes(
    const MlDsaParams& selected) {
  return selected.lambda_bits / 4;
}

[[nodiscard]] constexpr MlDsaDecomposition ml_dsa_power2round(
    const MlDsaParams& selected,
    std::int64_t r) {
  const auto base = ml_dsa_power2round_base(selected);
  const auto r_plus = ml_dsa_positive_mod(r, selected.q);
  const auto r0 = ml_dsa_centered_mod(r_plus, base);
  return {(r_plus - r0) / base, r0};
}

[[nodiscard]] constexpr MlDsaDecomposition ml_dsa_power2round(
    MlDsaParameterSet parameter_set,
    std::int64_t r) {
  return ml_dsa_power2round(params(parameter_set), r);
}

[[nodiscard]] constexpr MlDsaDecomposition ml_dsa_decompose(
    const MlDsaParams& selected,
    std::int64_t r) {
  const auto alpha = ml_dsa_decompose_base(selected);
  const auto r_plus = ml_dsa_positive_mod(r, selected.q);
  auto r0 = ml_dsa_centered_mod(r_plus, alpha);
  if (r_plus - r0 == selected.q - 1) {
    return {0, r0 - 1};
  }
  return {(r_plus - r0) / alpha, r0};
}

[[nodiscard]] constexpr MlDsaDecomposition ml_dsa_decompose(
    MlDsaParameterSet parameter_set,
    std::int64_t r) {
  return ml_dsa_decompose(params(parameter_set), r);
}

[[nodiscard]] constexpr std::int32_t ml_dsa_high_bits(const MlDsaParams& selected,
                                                      std::int64_t r) {
  return ml_dsa_decompose(selected, r).high;
}

[[nodiscard]] constexpr std::int32_t ml_dsa_high_bits(MlDsaParameterSet parameter_set,
                                                      std::int64_t r) {
  return ml_dsa_high_bits(params(parameter_set), r);
}

[[nodiscard]] constexpr std::int32_t ml_dsa_low_bits(const MlDsaParams& selected,
                                                     std::int64_t r) {
  return ml_dsa_decompose(selected, r).low;
}

[[nodiscard]] constexpr std::int32_t ml_dsa_low_bits(MlDsaParameterSet parameter_set,
                                                     std::int64_t r) {
  return ml_dsa_low_bits(params(parameter_set), r);
}

[[nodiscard]] constexpr bool ml_dsa_make_hint(const MlDsaParams& selected,
                                              std::int64_t z,
                                              std::int64_t r) {
  return ml_dsa_high_bits(selected, r) != ml_dsa_high_bits(selected, r + z);
}

[[nodiscard]] constexpr bool ml_dsa_make_hint(MlDsaParameterSet parameter_set,
                                              std::int64_t z,
                                              std::int64_t r) {
  return ml_dsa_make_hint(params(parameter_set), z, r);
}

[[nodiscard]] constexpr std::int32_t ml_dsa_use_hint(const MlDsaParams& selected,
                                                     bool hint,
                                                     std::int64_t r) {
  const auto decomposition = ml_dsa_decompose(selected, r);
  if (!hint) {
    return decomposition.high;
  }

  const auto modulus = ml_dsa_high_bits_modulus(selected);
  if (decomposition.low > 0) {
    return ml_dsa_positive_mod(decomposition.high + 1, modulus);
  }
  return ml_dsa_positive_mod(decomposition.high - 1, modulus);
}

[[nodiscard]] constexpr std::int32_t ml_dsa_use_hint(MlDsaParameterSet parameter_set,
                                                     bool hint,
                                                     std::int64_t r) {
  return ml_dsa_use_hint(params(parameter_set), hint, r);
}

[[nodiscard]] inline MlDsaPolynomial ml_dsa_challenge_polynomial_from_bytes(
    MlDsaParameterSet parameter_set,
    std::span<const std::uint8_t> bytes) {
  return ml_dsa_sample_in_ball_from_bytes(params(parameter_set).tau, bytes);
}

}  // namespace pqcore
