#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include "pqcore/field.hpp"
#include "pqcore/polynomial.hpp"

namespace pqcore {

inline constexpr std::size_t kMlKemNttDegree = 256;
inline constexpr std::int32_t kMlKemNttRoot = 17;
inline constexpr std::int32_t kMlKemInverseNttScale = 3303;

class MlKemNttPolynomial {
 public:
  using Coefficient = MlKemField;
  using Coefficients = std::array<Coefficient, kMlKemNttDegree>;

  constexpr MlKemNttPolynomial() = default;
  explicit constexpr MlKemNttPolynomial(Coefficients coefficients)
      : coefficients_(coefficients) {}

  [[nodiscard]] static constexpr std::size_t degree() noexcept { return kMlKemNttDegree; }
  [[nodiscard]] static constexpr std::int32_t modulus() noexcept { return MlKemField::modulus(); }

  [[nodiscard]] constexpr const Coefficients& coefficients() const noexcept {
    return coefficients_;
  }

  [[nodiscard]] constexpr Coefficient coeff_at(std::size_t index) const {
    if (index >= kMlKemNttDegree) {
      throw std::out_of_range{"NTT coefficient index out of range"};
    }
    return coefficients_[index];
  }

 private:
  Coefficients coefficients_{};
};

[[nodiscard]] constexpr std::size_t bit_reverse_7(std::size_t value) {
  if (value >= 128) {
    throw std::invalid_argument{"7-bit reversal input must be in [0, 127]"};
  }

  std::size_t result = 0;
  for (std::size_t bit = 0; bit < 7; ++bit) {
    result = (result << 1) | (value & 1u);
    value >>= 1;
  }
  return result;
}

[[nodiscard]] constexpr std::int32_t ml_kem_mod_pow(std::int32_t base,
                                                    std::size_t exponent) {
  std::int64_t result = 1;
  std::int64_t factor = MlKemField{base}.value();
  while (exponent > 0) {
    if ((exponent & 1u) != 0) {
      result = (result * factor) % MlKemField::modulus();
    }
    factor = (factor * factor) % MlKemField::modulus();
    exponent >>= 1;
  }
  return static_cast<std::int32_t>(result);
}

[[nodiscard]] constexpr MlKemField ml_kem_ntt_zeta(std::size_t index) {
  return MlKemField{ml_kem_mod_pow(kMlKemNttRoot, bit_reverse_7(index))};
}

[[nodiscard]] constexpr MlKemField ml_kem_ntt_gamma(std::size_t index) {
  return MlKemField{ml_kem_mod_pow(kMlKemNttRoot, (2 * bit_reverse_7(index)) + 1)};
}

[[nodiscard]] inline MlKemNttPolynomial ml_kem_ntt(const MlKemPolynomial& polynomial) {
  MlKemNttPolynomial::Coefficients coefficients{};
  for (std::size_t index = 0; index < kMlKemNttDegree; ++index) {
    coefficients[index] = polynomial.coeff_at(index);
  }

  std::size_t zeta_index = 1;
  for (std::size_t len = 128; len >= 2; len /= 2) {
    for (std::size_t start = 0; start < kMlKemNttDegree; start += 2 * len) {
      const auto zeta = ml_kem_ntt_zeta(zeta_index);
      ++zeta_index;
      for (std::size_t j = start; j < start + len; ++j) {
        const auto term = zeta * coefficients[j + len];
        coefficients[j + len] = coefficients[j] - term;
        coefficients[j] += term;
      }
    }
  }

  return MlKemNttPolynomial{coefficients};
}

[[nodiscard]] inline MlKemPolynomial ml_kem_inverse_ntt(const MlKemNttPolynomial& polynomial) {
  auto coefficients = polynomial.coefficients();

  std::size_t zeta_index = 127;
  for (std::size_t len = 2; len <= 128; len *= 2) {
    for (std::size_t start = 0; start < kMlKemNttDegree; start += 2 * len) {
      const auto zeta = ml_kem_ntt_zeta(zeta_index);
      --zeta_index;
      for (std::size_t j = start; j < start + len; ++j) {
        const auto term = coefficients[j];
        coefficients[j] = term + coefficients[j + len];
        coefficients[j + len] = zeta * (coefficients[j + len] - term);
      }
    }
  }

  std::array<std::int32_t, kMlKemNttDegree> result{};
  const auto scale = MlKemField{kMlKemInverseNttScale};
  for (std::size_t index = 0; index < kMlKemNttDegree; ++index) {
    result[index] = (coefficients[index] * scale).value();
  }
  return MlKemPolynomial{result};
}

[[nodiscard]] constexpr std::array<MlKemField, 2> ml_kem_ntt_base_case_multiply(
    MlKemField a0,
    MlKemField a1,
    MlKemField b0,
    MlKemField b1,
    MlKemField gamma) {
  return {a0 * b0 + a1 * b1 * gamma, a0 * b1 + a1 * b0};
}

[[nodiscard]] inline MlKemNttPolynomial ml_kem_ntt_multiply(
    const MlKemNttPolynomial& lhs,
    const MlKemNttPolynomial& rhs) {
  MlKemNttPolynomial::Coefficients coefficients{};
  for (std::size_t block = 0; block < 128; ++block) {
    const auto product = ml_kem_ntt_base_case_multiply(
        lhs.coeff_at(2 * block),
        lhs.coeff_at((2 * block) + 1),
        rhs.coeff_at(2 * block),
        rhs.coeff_at((2 * block) + 1),
        ml_kem_ntt_gamma(block));
    coefficients[2 * block] = product[0];
    coefficients[(2 * block) + 1] = product[1];
  }
  return MlKemNttPolynomial{coefficients};
}

}  // namespace pqcore

