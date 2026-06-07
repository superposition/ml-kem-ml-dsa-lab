#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include "pqcore/field.hpp"

namespace pqcore {

template <std::size_t N, std::int32_t Q>
class Polynomial {
  static_assert(N > 0, "Polynomial degree must be positive");

 public:
  using Coefficient = ModQ<Q>;
  using Coefficients = std::array<Coefficient, N>;

  constexpr Polynomial() = default;

  constexpr explicit Polynomial(std::array<std::int32_t, N> coefficients) {
    for (std::size_t index = 0; index < N; ++index) {
      coefficients_[index] = Coefficient{coefficients[index]};
    }
  }

  [[nodiscard]] static constexpr std::size_t degree() noexcept { return N; }
  [[nodiscard]] static constexpr std::int32_t modulus() noexcept { return Q; }

  [[nodiscard]] static constexpr Polynomial zero() { return Polynomial{}; }

  [[nodiscard]] static constexpr Polynomial one() {
    Polynomial result;
    result.coefficients_[0] = Coefficient{1};
    return result;
  }

  [[nodiscard]] constexpr const Coefficients& coefficients() const noexcept {
    return coefficients_;
  }

  [[nodiscard]] constexpr Coefficient coeff_at(std::size_t index) const {
    if (index >= N) {
      throw std::out_of_range{"polynomial coefficient index out of range"};
    }
    return coefficients_[index];
  }

  [[nodiscard]] constexpr Polynomial add(const Polynomial& rhs) const noexcept {
    Polynomial result;
    for (std::size_t index = 0; index < N; ++index) {
      result.coefficients_[index] = coefficients_[index] + rhs.coefficients_[index];
    }
    return result;
  }

  [[nodiscard]] constexpr Polynomial sub(const Polynomial& rhs) const noexcept {
    Polynomial result;
    for (std::size_t index = 0; index < N; ++index) {
      result.coefficients_[index] = coefficients_[index] - rhs.coefficients_[index];
    }
    return result;
  }

  [[nodiscard]] constexpr Polynomial schoolbook_mul_negacyclic(const Polynomial& rhs) const noexcept {
    Polynomial result;
    for (std::size_t i = 0; i < N; ++i) {
      for (std::size_t j = 0; j < N; ++j) {
        const auto term = coefficients_[i] * rhs.coefficients_[j];
        const auto degree = i + j;
        if (degree < N) {
          result.coefficients_[degree] += term;
        } else {
          result.coefficients_[degree - N] -= term;
        }
      }
    }
    return result;
  }

 private:
  Coefficients coefficients_{};
};

using MlKemPolynomial = Polynomial<256, 3329>;
using MlDsaPolynomial = Polynomial<256, 8380417>;

}  // namespace pqcore
