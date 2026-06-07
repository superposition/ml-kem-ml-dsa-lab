#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

#include "pqcore/encoding.hpp"
#include "pqcore/ntt.hpp"
#include "pqcore/params.hpp"
#include "pqcore/polynomial.hpp"
#include "pqcore/sampling.hpp"

namespace pqcore {

inline constexpr std::size_t kMlKem512K = 2;
inline constexpr std::size_t kMlKem512PublicKeyBytes = 800;
inline constexpr std::size_t kMlKem512SecretKeyBytes = 768;
inline constexpr std::size_t kMlKem512CiphertextBytes = 768;

using MlKemPkeNttVector = std::array<MlKemNttPolynomial, kMlKem512K>;
using MlKemPkePolyVector = std::array<MlKemPolynomial, kMlKem512K>;
using MlKemPkeMatrix = std::array<std::array<MlKemNttPolynomial, kMlKem512K>, kMlKem512K>;

struct MlKemPkeKeyPair {
  std::vector<std::uint8_t> encryption_key;
  std::vector<std::uint8_t> decryption_key;
};

struct MlKemPkeCiphertext {
  std::vector<std::uint8_t> bytes;
};

[[nodiscard]] inline std::vector<std::uint8_t> byte_slice(
    std::span<const std::uint8_t> bytes,
    std::size_t offset,
    std::size_t length) {
  if (offset > bytes.size() || length > bytes.size() - offset) {
    throw std::invalid_argument{"byte slice is out of range"};
  }
  return std::vector<std::uint8_t>(bytes.begin() + static_cast<std::ptrdiff_t>(offset),
                                   bytes.begin() + static_cast<std::ptrdiff_t>(offset + length));
}

[[nodiscard]] inline MlKemCoefficientArray coefficients_from_ntt(
    const MlKemNttPolynomial& polynomial) {
  MlKemCoefficientArray coefficients{};
  for (std::size_t index = 0; index < coefficients.size(); ++index) {
    coefficients[index] = polynomial.coeff_at(index).value();
  }
  return coefficients;
}

[[nodiscard]] inline MlKemCoefficientArray coefficients_from_polynomial(
    const MlKemPolynomial& polynomial) {
  MlKemCoefficientArray coefficients{};
  for (std::size_t index = 0; index < coefficients.size(); ++index) {
    coefficients[index] = polynomial.coeff_at(index).value();
  }
  return coefficients;
}

[[nodiscard]] inline MlKemNttPolynomial ml_kem_ntt_add(const MlKemNttPolynomial& lhs,
                                                       const MlKemNttPolynomial& rhs) {
  MlKemNttPolynomial::Coefficients coefficients{};
  for (std::size_t index = 0; index < coefficients.size(); ++index) {
    coefficients[index] = lhs.coeff_at(index) + rhs.coeff_at(index);
  }
  return MlKemNttPolynomial{coefficients};
}

[[nodiscard]] inline MlKemPolynomial ml_kem_pke_decompress_poly(
    const MlKemCoefficientArray& compressed,
    std::size_t bit_width) {
  std::array<std::int32_t, MlKemPolynomial::degree()> coefficients{};
  for (std::size_t index = 0; index < coefficients.size(); ++index) {
    coefficients[index] = ml_kem_decompress(compressed[index], bit_width).value();
  }
  return MlKemPolynomial{coefficients};
}

[[nodiscard]] inline MlKemCoefficientArray ml_kem_pke_compress_poly(
    const MlKemPolynomial& polynomial,
    std::size_t bit_width) {
  MlKemCoefficientArray compressed{};
  for (std::size_t index = 0; index < compressed.size(); ++index) {
    compressed[index] = ml_kem_compress(polynomial.coeff_at(index), bit_width);
  }
  return compressed;
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_kem_pke_encode_ntt_vector(
    const MlKemPkeNttVector& vector) {
  std::vector<std::uint8_t> encoded;
  encoded.reserve(kMlKem512K * ml_kem_byte_encoded_length(12));
  for (const auto& polynomial : vector) {
    const auto chunk = ml_kem_byte_encode(coefficients_from_ntt(polynomial), 12);
    encoded.insert(encoded.end(), chunk.begin(), chunk.end());
  }
  return encoded;
}

[[nodiscard]] inline MlKemPkeNttVector ml_kem_pke_decode_ntt_vector(
    std::span<const std::uint8_t> bytes) {
  const auto chunk_size = ml_kem_byte_encoded_length(12);
  if (bytes.size() != kMlKem512K * chunk_size) {
    throw std::invalid_argument{"ML-KEM-512 NTT vector encoding has the wrong length"};
  }

  MlKemPkeNttVector vector{};
  for (std::size_t index = 0; index < kMlKem512K; ++index) {
    const auto chunk = byte_slice(bytes, index * chunk_size, chunk_size);
    const auto decoded = ml_kem_byte_decode(chunk, 12);
    MlKemNttPolynomial::Coefficients coefficients{};
    for (std::size_t coefficient_index = 0; coefficient_index < coefficients.size();
         ++coefficient_index) {
      coefficients[coefficient_index] = MlKemField{decoded[coefficient_index]};
    }
    vector[index] = MlKemNttPolynomial{coefficients};
  }
  return vector;
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_kem_pke_encode_compressed_poly_vector(
    const MlKemPkePolyVector& vector,
    std::size_t bit_width) {
  std::vector<std::uint8_t> encoded;
  encoded.reserve(kMlKem512K * ml_kem_byte_encoded_length(bit_width));
  for (const auto& polynomial : vector) {
    const auto chunk = ml_kem_byte_encode(ml_kem_pke_compress_poly(polynomial, bit_width),
                                          bit_width);
    encoded.insert(encoded.end(), chunk.begin(), chunk.end());
  }
  return encoded;
}

[[nodiscard]] inline MlKemPkePolyVector ml_kem_pke_decode_compressed_poly_vector(
    std::span<const std::uint8_t> bytes,
    std::size_t bit_width) {
  const auto chunk_size = ml_kem_byte_encoded_length(bit_width);
  if (bytes.size() != kMlKem512K * chunk_size) {
    throw std::invalid_argument{"ML-KEM-512 compressed vector has the wrong length"};
  }

  MlKemPkePolyVector vector{};
  for (std::size_t index = 0; index < kMlKem512K; ++index) {
    const auto chunk = byte_slice(bytes, index * chunk_size, chunk_size);
    vector[index] = ml_kem_pke_decompress_poly(ml_kem_byte_decode(chunk, bit_width), bit_width);
  }
  return vector;
}

[[nodiscard]] inline MlKemPolynomial ml_kem_pke_decode_compressed_poly(
    std::span<const std::uint8_t> bytes,
    std::size_t bit_width) {
  if (bytes.size() != ml_kem_byte_encoded_length(bit_width)) {
    throw std::invalid_argument{"ML-KEM compressed polynomial has the wrong length"};
  }
  return ml_kem_pke_decompress_poly(
      ml_kem_byte_decode(std::vector<std::uint8_t>(bytes.begin(), bytes.end()), bit_width),
      bit_width);
}

#ifdef PQCORE_ENABLE_TEST_SAMPLING
[[nodiscard]] inline std::array<MlKemSeed, 2> ml_kem_pke_expand_keygen_seeds_512_test(
    const MlKemSeed& keygen_seed) {
  const auto expanded =
      deterministic_test_expand(keygen_seed, "ML-KEM-PKE-G-TEST", kMlKem512K, 64);
  std::array<MlKemSeed, 2> seeds{};
  std::copy_n(expanded.begin(), kMlKemSeedBytes, seeds[0].begin());
  std::copy_n(expanded.begin() + kMlKemSeedBytes, kMlKemSeedBytes, seeds[1].begin());
  return seeds;
}

[[nodiscard]] inline MlKemSampleNttSeed ml_kem_pke_matrix_seed(const MlKemSeed& rho,
                                                               std::uint8_t column,
                                                               std::uint8_t row) {
  MlKemSampleNttSeed seed{};
  std::copy(rho.begin(), rho.end(), seed.begin());
  seed[32] = column;
  seed[33] = row;
  return seed;
}

[[nodiscard]] inline MlKemPkeMatrix ml_kem_pke_generate_matrix_512_test(const MlKemSeed& rho) {
  MlKemPkeMatrix matrix{};
  for (std::size_t row = 0; row < kMlKem512K; ++row) {
    for (std::size_t column = 0; column < kMlKem512K; ++column) {
      matrix[row][column] = ml_kem_sample_ntt_test(
          ml_kem_pke_matrix_seed(rho, static_cast<std::uint8_t>(column),
                                 static_cast<std::uint8_t>(row)));
    }
  }
  return matrix;
}

[[nodiscard]] inline MlKemPolynomial ml_kem_pke_sample_noise_poly_512_test(
    const MlKemSeed& seed,
    std::size_t eta,
    std::uint8_t nonce) {
  return ml_kem_sample_poly_cbd(eta, ml_kem_prf_test(seed, nonce, eta));
}

[[nodiscard]] inline MlKemPkePolyVector ml_kem_pke_sample_noise_vector_512_test(
    const MlKemSeed& seed,
    std::size_t eta,
    std::uint8_t first_nonce) {
  MlKemPkePolyVector vector{};
  for (std::size_t index = 0; index < kMlKem512K; ++index) {
    vector[index] = ml_kem_pke_sample_noise_poly_512_test(
        seed, eta, static_cast<std::uint8_t>(first_nonce + index));
  }
  return vector;
}

[[nodiscard]] inline MlKemPkeNttVector ml_kem_pke_ntt_vector(
    const MlKemPkePolyVector& vector) {
  MlKemPkeNttVector transformed{};
  for (std::size_t index = 0; index < kMlKem512K; ++index) {
    transformed[index] = ml_kem_ntt(vector[index]);
  }
  return transformed;
}

[[nodiscard]] inline MlKemPkeKeyPair ml_kem_pke_keygen_512_test(const MlKemSeed& keygen_seed) {
  const auto expanded = ml_kem_pke_expand_keygen_seeds_512_test(keygen_seed);
  const auto& rho = expanded[0];
  const auto& sigma = expanded[1];

  const auto matrix = ml_kem_pke_generate_matrix_512_test(rho);
  const auto secret = ml_kem_pke_sample_noise_vector_512_test(sigma, 3, 0);
  const auto error = ml_kem_pke_sample_noise_vector_512_test(sigma, 3, 2);
  const auto secret_ntt = ml_kem_pke_ntt_vector(secret);
  const auto error_ntt = ml_kem_pke_ntt_vector(error);

  MlKemPkeNttVector public_ntt{};
  for (std::size_t row = 0; row < kMlKem512K; ++row) {
    auto sum = error_ntt[row];
    for (std::size_t column = 0; column < kMlKem512K; ++column) {
      sum = ml_kem_ntt_add(sum, ml_kem_ntt_multiply(matrix[row][column], secret_ntt[column]));
    }
    public_ntt[row] = sum;
  }

  auto public_key = ml_kem_pke_encode_ntt_vector(public_ntt);
  public_key.insert(public_key.end(), rho.begin(), rho.end());
  return {public_key, ml_kem_pke_encode_ntt_vector(secret_ntt)};
}

[[nodiscard]] inline MlKemPolynomial ml_kem_pke_message_to_polynomial(
    const std::vector<std::uint8_t>& message) {
  if (message.size() != 32) {
    throw std::invalid_argument{"ML-KEM-512 PKE message must be 32 bytes"};
  }
  return ml_kem_pke_decompress_poly(ml_kem_byte_decode(message, 1), 1);
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_kem_pke_polynomial_to_message(
    const MlKemPolynomial& polynomial) {
  return ml_kem_byte_encode(ml_kem_pke_compress_poly(polynomial, 1), 1);
}

[[nodiscard]] inline MlKemPkeCiphertext ml_kem_pke_encrypt_512_test(
    const std::vector<std::uint8_t>& encryption_key,
    const std::vector<std::uint8_t>& message,
    const MlKemSeed& coins) {
  if (encryption_key.size() != kMlKem512PublicKeyBytes) {
    throw std::invalid_argument{"ML-KEM-512 PKE encryption key has the wrong length"};
  }
  if (message.size() != 32) {
    throw std::invalid_argument{"ML-KEM-512 PKE message must be 32 bytes"};
  }

  const auto encoded_public = byte_slice(encryption_key, 0, kMlKem512K * 384);
  const auto public_ntt = ml_kem_pke_decode_ntt_vector(encoded_public);
  MlKemSeed rho{};
  std::copy_n(encryption_key.begin() + static_cast<std::ptrdiff_t>(kMlKem512K * 384),
              kMlKemSeedBytes, rho.begin());

  const auto matrix = ml_kem_pke_generate_matrix_512_test(rho);
  const auto y = ml_kem_pke_sample_noise_vector_512_test(coins, 3, 0);
  const auto e1 = ml_kem_pke_sample_noise_vector_512_test(coins, 2, 2);
  const auto e2 = ml_kem_pke_sample_noise_poly_512_test(coins, 2, 4);
  const auto y_ntt = ml_kem_pke_ntt_vector(y);

  MlKemPkePolyVector u{};
  for (std::size_t column = 0; column < kMlKem512K; ++column) {
    MlKemNttPolynomial sum;
    for (std::size_t row = 0; row < kMlKem512K; ++row) {
      sum = ml_kem_ntt_add(sum, ml_kem_ntt_multiply(matrix[row][column], y_ntt[row]));
    }
    u[column] = ml_kem_inverse_ntt(sum).add(e1[column]);
  }

  MlKemNttPolynomial v_sum;
  for (std::size_t index = 0; index < kMlKem512K; ++index) {
    v_sum = ml_kem_ntt_add(v_sum, ml_kem_ntt_multiply(public_ntt[index], y_ntt[index]));
  }
  const auto mu = ml_kem_pke_message_to_polynomial(message);
  const auto v = ml_kem_inverse_ntt(v_sum).add(e2).add(mu);

  auto ciphertext = ml_kem_pke_encode_compressed_poly_vector(u, 10);
  const auto encoded_v = ml_kem_byte_encode(ml_kem_pke_compress_poly(v, 4), 4);
  ciphertext.insert(ciphertext.end(), encoded_v.begin(), encoded_v.end());
  return {ciphertext};
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_kem_pke_decrypt_512(
    const std::vector<std::uint8_t>& decryption_key,
    const std::vector<std::uint8_t>& ciphertext) {
  if (decryption_key.size() != kMlKem512SecretKeyBytes) {
    throw std::invalid_argument{"ML-KEM-512 PKE decryption key has the wrong length"};
  }
  if (ciphertext.size() != kMlKem512CiphertextBytes) {
    throw std::invalid_argument{"ML-KEM-512 PKE ciphertext has the wrong length"};
  }

  const auto c1_size = kMlKem512K * ml_kem_byte_encoded_length(10);
  const auto encoded_u = byte_slice(ciphertext, 0, c1_size);
  const auto encoded_v = byte_slice(ciphertext, c1_size, ml_kem_byte_encoded_length(4));
  const auto u = ml_kem_pke_decode_compressed_poly_vector(encoded_u, 10);
  const auto v = ml_kem_pke_decode_compressed_poly(encoded_v, 4);
  const auto secret_ntt = ml_kem_pke_decode_ntt_vector(decryption_key);

  MlKemNttPolynomial product_sum;
  for (std::size_t index = 0; index < kMlKem512K; ++index) {
    product_sum = ml_kem_ntt_add(product_sum,
                                 ml_kem_ntt_multiply(secret_ntt[index], ml_kem_ntt(u[index])));
  }
  const auto w = v.sub(ml_kem_inverse_ntt(product_sum));
  return ml_kem_pke_polynomial_to_message(w);
}
#else
[[nodiscard]] inline MlKemPkeKeyPair ml_kem_pke_keygen_512_test(const MlKemSeed& keygen_seed) {
  (void)keygen_seed;
  throw std::logic_error{"ML-KEM-512 PKE test keygen requires PQCORE_ENABLE_TEST_SAMPLING"};
}

[[nodiscard]] inline MlKemPkeCiphertext ml_kem_pke_encrypt_512_test(
    const std::vector<std::uint8_t>& encryption_key,
    const std::vector<std::uint8_t>& message,
    const MlKemSeed& coins) {
  (void)encryption_key;
  (void)message;
  (void)coins;
  throw std::logic_error{"ML-KEM-512 PKE test encryption requires PQCORE_ENABLE_TEST_SAMPLING"};
}

[[nodiscard]] inline std::vector<std::uint8_t> ml_kem_pke_decrypt_512(
    const std::vector<std::uint8_t>& decryption_key,
    const std::vector<std::uint8_t>& ciphertext) {
  (void)decryption_key;
  (void)ciphertext;
  throw std::logic_error{"ML-KEM-512 PKE test decryption requires PQCORE_ENABLE_TEST_SAMPLING"};
}
#endif

}  // namespace pqcore
