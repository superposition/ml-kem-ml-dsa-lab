#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "pqcore/field.hpp"
#include "pqcore/ntt.hpp"
#include "pqcore/polynomial.hpp"

namespace pqcore {

inline constexpr std::size_t kMlKemSeedBytes = 32;
inline constexpr std::size_t kMlKemSampleNttInputBytes = 34;
inline constexpr std::int32_t kMlDsaQ = 8380417;

using MlKemSeed = std::array<std::uint8_t, kMlKemSeedBytes>;
using MlKemSampleNttSeed = std::array<std::uint8_t, kMlKemSampleNttInputBytes>;

#ifdef PQCORE_ENABLE_TEST_SAMPLING
inline constexpr bool kDeterministicTestSamplingEnabled = true;
#else
inline constexpr bool kDeterministicTestSamplingEnabled = false;
#endif

[[nodiscard]] inline std::vector<std::uint8_t> production_random_bytes(std::size_t output_bytes) {
  (void)output_bytes;
  throw std::logic_error{
      "production entropy source is not configured; see docs/production-readiness.md"};
}

[[nodiscard]] inline std::vector<std::uint8_t> bytes_to_bits(std::span<const std::uint8_t> bytes) {
  std::vector<std::uint8_t> bits;
  bits.reserve(bytes.size() * 8);
  for (const auto byte : bytes) {
    for (std::size_t bit = 0; bit < 8; ++bit) {
      bits.push_back(static_cast<std::uint8_t>((byte >> bit) & 1u));
    }
  }
  return bits;
}

[[nodiscard]] constexpr std::size_t ml_kem_prf_output_bytes(std::size_t eta) {
  if (eta != 2 && eta != 3) {
    throw std::invalid_argument{"ML-KEM PRF eta must be 2 or 3"};
  }
  return 64 * eta;
}

[[nodiscard]] inline MlKemNttPolynomial ml_kem_sample_ntt_from_bytes(
    std::span<const std::uint8_t> bytes) {
  MlKemNttPolynomial::Coefficients coefficients{};

  std::size_t byte_index = 0;
  std::size_t coefficient_index = 0;
  while (coefficient_index < coefficients.size()) {
    if (byte_index + 3 > bytes.size()) {
      throw std::invalid_argument{"not enough bytes to complete ML-KEM SampleNTT"};
    }

    const auto c0 = static_cast<std::int32_t>(bytes[byte_index]);
    const auto c1 = static_cast<std::int32_t>(bytes[byte_index + 1]);
    const auto c2 = static_cast<std::int32_t>(bytes[byte_index + 2]);
    byte_index += 3;

    const auto d1 = c0 + 256 * (c1 & 0x0f);
    const auto d2 = (c1 >> 4) + 16 * c2;

    if (d1 < MlKemField::modulus()) {
      coefficients[coefficient_index] = MlKemField{d1};
      ++coefficient_index;
    }
    if (d2 < MlKemField::modulus() && coefficient_index < coefficients.size()) {
      coefficients[coefficient_index] = MlKemField{d2};
      ++coefficient_index;
    }
  }

  return MlKemNttPolynomial{coefficients};
}

[[nodiscard]] inline MlKemPolynomial ml_kem_sample_poly_cbd(
    std::size_t eta,
    std::span<const std::uint8_t> bytes) {
  if (eta != 2 && eta != 3) {
    throw std::invalid_argument{"ML-KEM CBD eta must be 2 or 3"};
  }
  if (bytes.size() != 64 * eta) {
    throw std::invalid_argument{"ML-KEM CBD input has the wrong length"};
  }

  const auto bits = bytes_to_bits(bytes);
  std::array<std::int32_t, MlKemPolynomial::degree()> coefficients{};

  for (std::size_t coefficient_index = 0; coefficient_index < coefficients.size();
       ++coefficient_index) {
    std::int32_t x = 0;
    std::int32_t y = 0;
    const auto bit_offset = 2 * coefficient_index * eta;
    for (std::size_t bit = 0; bit < eta; ++bit) {
      x += bits[bit_offset + bit];
      y += bits[bit_offset + eta + bit];
    }
    coefficients[coefficient_index] = x - y;
  }

  return MlKemPolynomial{coefficients};
}

[[nodiscard]] constexpr std::optional<std::int32_t> ml_dsa_coeff_from_three_bytes(
    std::uint8_t b0,
    std::uint8_t b1,
    std::uint8_t b2) {
  const auto masked_b2 = static_cast<std::int32_t>(b2 & 0x7f);
  const auto value =
      (65536 * masked_b2) + (256 * static_cast<std::int32_t>(b1)) +
      static_cast<std::int32_t>(b0);
  if (value < kMlDsaQ) {
    return value;
  }
  return std::nullopt;
}

[[nodiscard]] constexpr std::optional<std::int32_t> ml_dsa_coeff_from_half_byte(
    std::uint8_t value,
    std::size_t eta) {
  if (value > 15) {
    throw std::invalid_argument{"half-byte coefficient input must be in [0, 15]"};
  }
  if (eta == 2 && value < 15) {
    return 2 - static_cast<std::int32_t>(value % 5);
  }
  if (eta == 4 && value < 9) {
    return 4 - static_cast<std::int32_t>(value);
  }
  if (eta != 2 && eta != 4) {
    throw std::invalid_argument{"ML-DSA bounded sampler eta must be 2 or 4"};
  }
  return std::nullopt;
}

[[nodiscard]] inline MlDsaPolynomial ml_dsa_rej_bounded_poly_from_bytes(
    std::size_t eta,
    std::span<const std::uint8_t> bytes) {
  if (eta != 2 && eta != 4) {
    throw std::invalid_argument{"ML-DSA bounded sampler eta must be 2 or 4"};
  }

  std::array<std::int32_t, MlDsaPolynomial::degree()> coefficients{};
  std::size_t coefficient_index = 0;
  for (const auto byte : bytes) {
    const auto lower = ml_dsa_coeff_from_half_byte(byte & 0x0f, eta);
    if (lower.has_value()) {
      coefficients[coefficient_index] = *lower;
      ++coefficient_index;
      if (coefficient_index == coefficients.size()) {
        return MlDsaPolynomial{coefficients};
      }
    }

    const auto upper = ml_dsa_coeff_from_half_byte(byte >> 4, eta);
    if (upper.has_value()) {
      coefficients[coefficient_index] = *upper;
      ++coefficient_index;
      if (coefficient_index == coefficients.size()) {
        return MlDsaPolynomial{coefficients};
      }
    }
  }

  throw std::invalid_argument{"not enough bytes to complete ML-DSA RejBoundedPoly"};
}

[[nodiscard]] inline MlDsaPolynomial ml_dsa_sample_in_ball_from_bytes(
    std::size_t tau,
    std::span<const std::uint8_t> bytes) {
  if (tau > 64) {
    throw std::invalid_argument{"ML-DSA SampleInBall tau must be at most 64"};
  }
  if (bytes.size() < 8) {
    throw std::invalid_argument{"ML-DSA SampleInBall requires sign bytes"};
  }

  const auto sign_bits = bytes_to_bits(bytes.subspan(0, 8));
  std::array<std::int32_t, MlDsaPolynomial::degree()> coefficients{};
  std::size_t byte_index = 8;

  for (std::size_t offset = 0; offset < tau; ++offset) {
    const auto i = 256 - tau + offset;
    std::uint8_t j = 0;
    do {
      if (byte_index >= bytes.size()) {
        throw std::invalid_argument{"not enough bytes to complete ML-DSA SampleInBall"};
      }
      j = bytes[byte_index];
      ++byte_index;
    } while (j > i);

    coefficients[i] = coefficients[j];
    coefficients[j] = sign_bits[offset] == 0 ? 1 : -1;
  }

  return MlDsaPolynomial{coefficients};
}

#ifdef PQCORE_ENABLE_TEST_SAMPLING
[[nodiscard]] constexpr std::uint64_t deterministic_test_mix64(std::uint64_t value) noexcept {
  value ^= value >> 30;
  value *= 0xbf58476d1ce4e5b9ULL;
  value ^= value >> 27;
  value *= 0x94d049bb133111ebULL;
  value ^= value >> 31;
  return value;
}

[[nodiscard]] inline std::vector<std::uint8_t> deterministic_test_expand(
    std::span<const std::uint8_t> seed,
    std::string_view domain,
    std::uint16_t nonce,
    std::size_t output_bytes) {
  std::uint64_t state = 0x6a09e667f3bcc909ULL;
  const auto absorb = [&state](std::uint8_t byte) {
    state ^= byte;
    state *= 0x100000001b3ULL;
    state = deterministic_test_mix64(state);
  };

  for (const auto value : domain) {
    absorb(static_cast<std::uint8_t>(value));
  }
  absorb(0);
  for (const auto value : seed) {
    absorb(value);
  }
  absorb(static_cast<std::uint8_t>(nonce & 0xffu));
  absorb(static_cast<std::uint8_t>(nonce >> 8));

  std::vector<std::uint8_t> output;
  output.reserve(output_bytes);
  for (std::uint64_t counter = 0; output.size() < output_bytes; ++counter) {
    const auto block = deterministic_test_mix64(
        state + 0x9e3779b97f4a7c15ULL * (counter + 1));
    for (std::size_t byte_index = 0; byte_index < 8 && output.size() < output_bytes;
         ++byte_index) {
      output.push_back(static_cast<std::uint8_t>(block >> (8 * byte_index)));
    }
  }

  return output;
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_kem_prf_test(
    const MlKemSeed& seed,
    std::uint8_t nonce,
    std::size_t eta) {
  return deterministic_test_expand(
      seed, "ML-KEM-PRF-TEST", nonce, ml_kem_prf_output_bytes(eta));
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_kem_xof_test(
    const MlKemSampleNttSeed& seed,
    std::size_t output_bytes) {
  const auto nonce = static_cast<std::uint16_t>(seed[32] | (seed[33] << 8));
  return deterministic_test_expand(seed, "ML-KEM-XOF-TEST", nonce, output_bytes);
}

[[nodiscard]] inline MlKemNttPolynomial ml_kem_sample_ntt_test(
    const MlKemSampleNttSeed& seed) {
  return ml_kem_sample_ntt_from_bytes(ml_kem_xof_test(seed, 3 * 1024));
}
#endif

}  // namespace pqcore

