#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "pqcore/encoding.hpp"
#include "pqcore/kem.hpp"

template <typename T>
concept StreamWritable = requires(std::ostream& stream, const T& value) {
  stream << value;
};

template <typename Fn>
void assert_throws_invalid_argument(Fn&& fn) {
  bool threw = false;
  try {
    fn();
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);
}

std::int32_t centered_distance(std::int32_t lhs, std::int32_t rhs) {
  const auto absolute = lhs > rhs ? lhs - rhs : rhs - lhs;
  return absolute > (pqcore::kMlKemQ / 2) ? pqcore::kMlKemQ - absolute : absolute;
}

int main() {
  static_assert(!StreamWritable<pqcore::SharedSecret>);
  static_assert(!StreamWritable<pqcore::MlKemKeyPair>);

  assert(pqcore::ml_kem_byte_encoded_length(1) == 32);
  assert(pqcore::ml_kem_byte_encoded_length(4) == 128);
  assert(pqcore::ml_kem_byte_encoded_length(10) == 320);
  assert(pqcore::ml_kem_byte_encoded_length(11) == 352);
  assert(pqcore::ml_kem_byte_encoded_length(12) == 384);

  const auto kem512 = pqcore::params(pqcore::MlKemParameterSet::MlKem512);
  const auto kem768 = pqcore::params(pqcore::MlKemParameterSet::MlKem768);
  const auto kem1024 = pqcore::params(pqcore::MlKemParameterSet::MlKem1024);
  assert(pqcore::ml_kem_public_key_encoded_bytes(pqcore::MlKemParameterSet::MlKem512) ==
         kem512.public_key_bytes);
  assert(pqcore::ml_kem_public_key_encoded_bytes(pqcore::MlKemParameterSet::MlKem768) ==
         kem768.public_key_bytes);
  assert(pqcore::ml_kem_public_key_encoded_bytes(pqcore::MlKemParameterSet::MlKem1024) ==
         kem1024.public_key_bytes);
  assert(pqcore::ml_kem_ciphertext_encoded_bytes(pqcore::MlKemParameterSet::MlKem512) ==
         kem512.ciphertext_bytes);
  assert(pqcore::ml_kem_ciphertext_encoded_bytes(pqcore::MlKemParameterSet::MlKem768) ==
         kem768.ciphertext_bytes);
  assert(pqcore::ml_kem_ciphertext_encoded_bytes(pqcore::MlKemParameterSet::MlKem1024) ==
         kem1024.ciphertext_bytes);

  pqcore::MlKemCoefficientArray four_bit_coefficients{};
  for (std::size_t index = 0; index < four_bit_coefficients.size(); ++index) {
    four_bit_coefficients[index] = static_cast<std::int32_t>(index % 16);
  }
  const auto encoded_four_bit = pqcore::ml_kem_byte_encode(four_bit_coefficients, 4);
  assert(encoded_four_bit.size() == 128);
  const auto decoded_four_bit = pqcore::ml_kem_byte_decode(encoded_four_bit, 4);
  assert(decoded_four_bit == four_bit_coefficients);

  pqcore::MlKemCoefficientArray twelve_bit_coefficients{};
  twelve_bit_coefficients[0] = 0;
  twelve_bit_coefficients[1] = 1;
  twelve_bit_coefficients[2] = 3328;
  twelve_bit_coefficients[255] = 1234;
  const auto encoded_twelve_bit = pqcore::ml_kem_byte_encode(twelve_bit_coefficients, 12);
  assert(encoded_twelve_bit.size() == 384);
  const auto decoded_twelve_bit = pqcore::ml_kem_byte_decode(encoded_twelve_bit, 12);
  assert(decoded_twelve_bit == twelve_bit_coefficients);

  std::vector<std::uint8_t> non_canonical_twelve_bit(384, 0);
  non_canonical_twelve_bit[0] = 0xff;
  non_canonical_twelve_bit[1] = 0x0f;
  const auto decoded_non_canonical = pqcore::ml_kem_byte_decode(non_canonical_twelve_bit, 12);
  assert(decoded_non_canonical[0] == 4095 % pqcore::kMlKemQ);

  assert_throws_invalid_argument([] {
    pqcore::MlKemCoefficientArray coefficients{};
    coefficients[0] = 16;
    (void)pqcore::ml_kem_byte_encode(coefficients, 4);
  });
  assert_throws_invalid_argument([] {
    pqcore::MlKemCoefficientArray coefficients{};
    coefficients[0] = 3329;
    (void)pqcore::ml_kem_byte_encode(coefficients, 12);
  });
  assert_throws_invalid_argument([] {
    (void)pqcore::ml_kem_byte_decode(std::vector<std::uint8_t>(127), 4);
  });
  assert_throws_invalid_argument([] {
    (void)pqcore::ml_kem_byte_encoded_length(0);
  });

  assert(pqcore::ml_kem_compress(pqcore::MlKemField{0}, 1) == 0);
  assert(pqcore::ml_kem_compress(pqcore::MlKemField{1664}, 1) == 1);
  assert(pqcore::ml_kem_compress(pqcore::MlKemField{3328}, 1) == 0);
  assert(pqcore::ml_kem_compress(pqcore::MlKemField{0}, 4) == 0);
  assert(pqcore::ml_kem_compress(pqcore::MlKemField{3328}, 4) == 0);
  assert(pqcore::ml_kem_decompress(0, 4).value() == 0);
  assert(pqcore::ml_kem_decompress(15, 4).value() == 3121);
  assert_throws_invalid_argument([] {
    (void)pqcore::ml_kem_decompress(16, 4);
  });

  for (std::int32_t compressed = 0; compressed < 16; ++compressed) {
    assert(pqcore::ml_kem_compress(pqcore::ml_kem_decompress(compressed, 4), 4) ==
           compressed);
  }

  for (std::int32_t compressed = 0; compressed < 1024; ++compressed) {
    assert(pqcore::ml_kem_compress(pqcore::ml_kem_decompress(compressed, 10), 10) ==
           compressed);
  }

  const auto bound_d4 = (pqcore::kMlKemQ + 16) / 32;
  for (std::int32_t value = 0; value < pqcore::kMlKemQ; ++value) {
    const auto decompressed =
        pqcore::ml_kem_decompress(pqcore::ml_kem_compress(pqcore::MlKemField{value}, 4), 4);
    assert(centered_distance(value, decompressed.value()) <= bound_d4);
  }

  const auto bound_d10 = (pqcore::kMlKemQ + 1024) / 2048;
  for (std::int32_t value = 0; value < pqcore::kMlKemQ; ++value) {
    const auto decompressed =
        pqcore::ml_kem_decompress(pqcore::ml_kem_compress(pqcore::MlKemField{value}, 10), 10);
    assert(centered_distance(value, decompressed.value()) <= bound_d10);
  }
}
