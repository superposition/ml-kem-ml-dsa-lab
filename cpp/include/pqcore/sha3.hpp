#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

namespace pqcore {

using Sha3Digest256 = std::array<std::uint8_t, 32>;
using Sha3Digest512 = std::array<std::uint8_t, 64>;

namespace detail {

[[nodiscard]] constexpr std::uint64_t rotl64(std::uint64_t value, unsigned int shift) noexcept {
  return shift == 0 ? value : ((value << shift) | (value >> (64 - shift)));
}

[[nodiscard]] constexpr std::uint64_t load64_le(std::span<const std::uint8_t, 8> bytes) noexcept {
  std::uint64_t value = 0;
  for (std::size_t index = 0; index < bytes.size(); ++index) {
    value |= static_cast<std::uint64_t>(bytes[index]) << (8 * index);
  }
  return value;
}

inline void store64_le(std::uint64_t value, std::span<std::uint8_t, 8> bytes) noexcept {
  for (std::size_t index = 0; index < bytes.size(); ++index) {
    bytes[index] = static_cast<std::uint8_t>(value >> (8 * index));
  }
}

inline void keccak_f1600(std::array<std::uint64_t, 25>& state) noexcept {
  static constexpr std::array<std::uint64_t, 24> round_constants{
      0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
      0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
      0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
      0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
      0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
      0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
      0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
      0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL};
  static constexpr std::array<unsigned int, 25> rotation_offsets{
      0,  1,  62, 28, 27,
      36, 44, 6,  55, 20,
      3,  10, 43, 25, 39,
      41, 45, 15, 21, 8,
      18, 2,  61, 56, 14};

  for (const auto round_constant : round_constants) {
    std::array<std::uint64_t, 5> column_parity{};
    for (std::size_t x = 0; x < 5; ++x) {
      column_parity[x] = state[x] ^ state[x + 5] ^ state[x + 10] ^
                         state[x + 15] ^ state[x + 20];
    }

    for (std::size_t x = 0; x < 5; ++x) {
      const auto mix = column_parity[(x + 4) % 5] ^ rotl64(column_parity[(x + 1) % 5], 1);
      for (std::size_t y = 0; y < 5; ++y) {
        state[x + 5 * y] ^= mix;
      }
    }

    std::array<std::uint64_t, 25> rotated{};
    for (std::size_t x = 0; x < 5; ++x) {
      for (std::size_t y = 0; y < 5; ++y) {
        rotated[y + 5 * ((2 * x + 3 * y) % 5)] =
            rotl64(state[x + 5 * y], rotation_offsets[x + 5 * y]);
      }
    }

    for (std::size_t y = 0; y < 5; ++y) {
      for (std::size_t x = 0; x < 5; ++x) {
        state[x + 5 * y] =
            rotated[x + 5 * y] ^
            ((~rotated[((x + 1) % 5) + 5 * y]) & rotated[((x + 2) % 5) + 5 * y]);
      }
    }

    state[0] ^= round_constant;
  }
}

[[nodiscard]] inline std::vector<std::uint8_t> keccak_sponge(
    std::size_t rate_bytes,
    std::span<const std::uint8_t> input,
    std::uint8_t domain_suffix,
    std::size_t output_bytes) {
  if (rate_bytes == 0 || rate_bytes % 8 != 0 || rate_bytes > 200) {
    throw std::invalid_argument{"invalid Keccak rate"};
  }

  std::array<std::uint64_t, 25> state{};
  std::size_t offset = 0;
  while (input.size() - offset >= rate_bytes) {
    for (std::size_t lane = 0; lane < rate_bytes / 8; ++lane) {
      state[lane] ^= load64_le(
          std::span<const std::uint8_t, 8>(input.data() + offset + lane * 8, 8));
    }
    keccak_f1600(state);
    offset += rate_bytes;
  }

  std::array<std::uint8_t, 200> block{};
  const auto remaining = input.size() - offset;
  for (std::size_t index = 0; index < remaining; ++index) {
    block[index] = input[offset + index];
  }
  block[remaining] ^= domain_suffix;
  block[rate_bytes - 1] ^= 0x80;
  for (std::size_t lane = 0; lane < rate_bytes / 8; ++lane) {
    state[lane] ^= load64_le(std::span<const std::uint8_t, 8>(block.data() + lane * 8, 8));
  }
  keccak_f1600(state);

  std::vector<std::uint8_t> output;
  output.reserve(output_bytes);
  std::array<std::uint8_t, 200> squeezed{};
  while (output.size() < output_bytes) {
    for (std::size_t lane = 0; lane < rate_bytes / 8; ++lane) {
      store64_le(state[lane], std::span<std::uint8_t, 8>(squeezed.data() + lane * 8, 8));
    }
    const auto take = std::min(rate_bytes, output_bytes - output.size());
    output.insert(output.end(), squeezed.begin(), squeezed.begin() + static_cast<std::ptrdiff_t>(take));
    if (output.size() < output_bytes) {
      keccak_f1600(state);
    }
  }
  return output;
}

}  // namespace detail

[[nodiscard]] inline Sha3Digest256 sha3_256(std::span<const std::uint8_t> input) {
  const auto bytes = detail::keccak_sponge(136, input, 0x06, 32);
  Sha3Digest256 digest{};
  std::copy(bytes.begin(), bytes.end(), digest.begin());
  return digest;
}

[[nodiscard]] inline Sha3Digest512 sha3_512(std::span<const std::uint8_t> input) {
  const auto bytes = detail::keccak_sponge(72, input, 0x06, 64);
  Sha3Digest512 digest{};
  std::copy(bytes.begin(), bytes.end(), digest.begin());
  return digest;
}

[[nodiscard]] inline std::vector<std::uint8_t> shake128(
    std::span<const std::uint8_t> input,
    std::size_t output_bytes) {
  return detail::keccak_sponge(168, input, 0x1f, output_bytes);
}

[[nodiscard]] inline std::vector<std::uint8_t> shake256(
    std::span<const std::uint8_t> input,
    std::size_t output_bytes) {
  return detail::keccak_sponge(136, input, 0x1f, output_bytes);
}

}  // namespace pqcore
