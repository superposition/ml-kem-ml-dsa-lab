#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "pqcore/field.hpp"
#include "pqcore/params.hpp"

namespace pqcore {

inline constexpr std::int32_t kMlKemQ = 3329;
inline constexpr std::size_t kMlKemCoefficientCount = 256;

using MlKemCoefficientArray = std::array<std::int32_t, kMlKemCoefficientCount>;

[[nodiscard]] constexpr std::size_t ml_kem_byte_encoded_length(std::size_t bit_width) {
  if (bit_width < 1 || bit_width > 12) {
    throw std::invalid_argument{"ML-KEM byte encoding width must be in [1, 12]"};
  }
  return 32 * bit_width;
}

[[nodiscard]] constexpr std::int32_t ml_kem_byte_encoding_modulus(std::size_t bit_width) {
  if (bit_width < 1 || bit_width > 12) {
    throw std::invalid_argument{"ML-KEM byte encoding width must be in [1, 12]"};
  }
  return bit_width == 12 ? kMlKemQ : static_cast<std::int32_t>(1u << bit_width);
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_kem_byte_encode(
    const MlKemCoefficientArray& coefficients,
    std::size_t bit_width) {
  const auto output_size = ml_kem_byte_encoded_length(bit_width);
  const auto modulus = ml_kem_byte_encoding_modulus(bit_width);
  std::vector<std::uint8_t> output(output_size, 0);

  for (std::size_t coefficient_index = 0; coefficient_index < coefficients.size();
       ++coefficient_index) {
    auto value = coefficients[coefficient_index];
    if (value < 0 || value >= modulus) {
      throw std::invalid_argument{"coefficient is outside the ML-KEM ByteEncode domain"};
    }

    for (std::size_t bit_index = 0; bit_index < bit_width; ++bit_index) {
      const auto flattened_bit_index = coefficient_index * bit_width + bit_index;
      if ((value & 1) != 0) {
        output[flattened_bit_index / 8] |=
            static_cast<std::uint8_t>(1u << (flattened_bit_index % 8));
      }
      value >>= 1;
    }
  }

  return output;
}

[[nodiscard]] inline MlKemCoefficientArray ml_kem_byte_decode(
    const std::vector<std::uint8_t>& bytes,
    std::size_t bit_width) {
  const auto expected_size = ml_kem_byte_encoded_length(bit_width);
  if (bytes.size() != expected_size) {
    throw std::invalid_argument{"ML-KEM ByteDecode input has the wrong length"};
  }

  const auto modulus = ml_kem_byte_encoding_modulus(bit_width);
  MlKemCoefficientArray coefficients{};

  for (std::size_t coefficient_index = 0; coefficient_index < coefficients.size();
       ++coefficient_index) {
    std::int32_t value = 0;
    for (std::size_t bit_index = 0; bit_index < bit_width; ++bit_index) {
      const auto flattened_bit_index = coefficient_index * bit_width + bit_index;
      const auto bit =
          (bytes[flattened_bit_index / 8] >> (flattened_bit_index % 8)) & 1u;
      value += static_cast<std::int32_t>(bit << bit_index);
    }
    coefficients[coefficient_index] = value % modulus;
  }

  return coefficients;
}

[[nodiscard]] constexpr std::int32_t ml_kem_compression_modulus(std::size_t bit_width) {
  if (bit_width < 1 || bit_width >= 12) {
    throw std::invalid_argument{"ML-KEM compression width must be in [1, 11]"};
  }
  return static_cast<std::int32_t>(1u << bit_width);
}

[[nodiscard]] constexpr std::int32_t ml_kem_compress(MlKemField value,
                                                     std::size_t bit_width) {
  const auto modulus = ml_kem_compression_modulus(bit_width);
  const auto numerator =
      static_cast<std::int64_t>(value.value()) * modulus + (kMlKemQ / 2);
  return static_cast<std::int32_t>((numerator / kMlKemQ) % modulus);
}

[[nodiscard]] constexpr MlKemField ml_kem_decompress(std::int32_t value,
                                                     std::size_t bit_width) {
  const auto modulus = ml_kem_compression_modulus(bit_width);
  if (value < 0 || value >= modulus) {
    throw std::invalid_argument{"compressed ML-KEM coefficient is outside its bit width"};
  }

  const auto numerator =
      static_cast<std::int64_t>(value) * kMlKemQ + (modulus / 2);
  return MlKemField{static_cast<std::int32_t>(numerator / modulus)};
}

[[nodiscard]] constexpr std::size_t ml_kem_public_key_encoded_bytes(
    MlKemParameterSet parameter_set) {
  const auto selected = params(parameter_set);
  return 32 + selected.k * ml_kem_byte_encoded_length(12);
}

[[nodiscard]] constexpr std::size_t ml_kem_ciphertext_encoded_bytes(
    MlKemParameterSet parameter_set) {
  const auto selected = params(parameter_set);
  return selected.k * ml_kem_byte_encoded_length(selected.du) +
         ml_kem_byte_encoded_length(selected.dv);
}

}  // namespace pqcore

