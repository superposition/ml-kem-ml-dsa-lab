#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "pqcore/sha3.hpp"

std::uint8_t hex_value(char value) {
  if (value >= '0' && value <= '9') {
    return static_cast<std::uint8_t>(value - '0');
  }
  if (value >= 'a' && value <= 'f') {
    return static_cast<std::uint8_t>(10 + value - 'a');
  }
  throw std::invalid_argument{"invalid lowercase hex digit"};
}

std::vector<std::uint8_t> hex_to_bytes(std::string_view hex) {
  if (hex.size() % 2 != 0) {
    throw std::invalid_argument{"hex string length must be even"};
  }
  std::vector<std::uint8_t> bytes;
  bytes.reserve(hex.size() / 2);
  for (std::size_t index = 0; index < hex.size(); index += 2) {
    bytes.push_back(static_cast<std::uint8_t>(
        (hex_value(hex[index]) << 4) | hex_value(hex[index + 1])));
  }
  return bytes;
}

template <typename Digest>
std::vector<std::uint8_t> digest_to_vector(const Digest& digest) {
  return std::vector<std::uint8_t>(digest.begin(), digest.end());
}

int main() {
  const std::vector<std::uint8_t> empty;
  const std::vector<std::uint8_t> abc{'a', 'b', 'c'};
  const std::vector<std::uint8_t> long_message(200, 'a');

  assert(digest_to_vector(pqcore::sha3_256(empty)) ==
         hex_to_bytes("a7ffc6f8bf1ed76651c14756a061d662f580ff4de43b49fa82d80a4b80f8434a"));
  assert(digest_to_vector(pqcore::sha3_256(abc)) ==
         hex_to_bytes("3a985da74fe225b2045c172d6bd390bd855f086e3e9d525b46bfe24511431532"));

  assert(digest_to_vector(pqcore::sha3_512(empty)) ==
         hex_to_bytes("a69f73cca23a9ac5c8b567dc185a756e97c982164fe25859e0d1dcc1475c80a615b"
                      "2123af1f5f94c11e3e9402c3ac558f500199d95b6d3e301758586281dcd26"));
  assert(digest_to_vector(pqcore::sha3_512(abc)) ==
         hex_to_bytes("b751850b1a57168a5693cd924b6b096e08f621827444f70d884f5d0240d2712e"
                      "10e116e9192af3c91a7ec57647e3934057340b4cf408d5a56592f8274eec53f0"));

  assert(pqcore::shake128(empty, 32) ==
         hex_to_bytes("7f9c2ba4e88f827d616045507605853ed73b8093f6efbc88eb1a6eacfa66ef26"));
  assert(pqcore::shake128(abc, 32) ==
         hex_to_bytes("5881092dd818bf5cf8a3ddb793fbcba74097d5c526a6d35f97b83351940f2cc8"));

  assert(pqcore::shake256(empty, 64) ==
         hex_to_bytes("46b9dd2b0ba88d13233b3feb743eeb243fcd52ea62b81b82b50c27646ed5762f"
                      "d75dc4ddd8c0f200cb05019d67b592f6fc821c49479ab48640292eacb3b7c4be"));
  assert(pqcore::shake256(abc, 64) ==
         hex_to_bytes("483366601360a8771c6863080cc4114d8db44530f8f1e1ee4f94ea37e78b5739"
                      "d5a15bef186a5386c75744c0527e1faa9f8726e462a12a4feb06bd8801e751e4"));

  assert(digest_to_vector(pqcore::sha3_256(long_message)) ==
         hex_to_bytes("cce34485baf2bf2aca99b94833892a4f52896d3d153f7b840cc4f9fe695f1387"));
  assert(digest_to_vector(pqcore::sha3_512(long_message)) ==
         hex_to_bytes("eae6c85c6904f11075de9f9d5e1064371d000510fa3d2d79d40cf9be34892fb0"
                      "1859d0a0234e138bcb0ad5c84f6c0dca226a414b0c9a2897cb695f5185fe36ec"));
  assert(pqcore::shake128(long_message, 200) ==
         hex_to_bytes("70ac9b97e891be583e08929ce4cce50d346b05f9597356d6af94d4643d2af3b6"
                      "7eb416f94f88a5339f507173ea86c5abff2e1d1087032ddc93e06467ef256c"
                      "277bf49fc94dc03497c52864bb83f1bf4ee8569bfc78474e5f82e8c99a74d5"
                      "ca2b1ec32bb54838959cd701350b3977e1e6f722884a6c701118df3e3174ad"
                      "2289440852d03657dfa0b96ac86fb29d88212c19390c0502a62f71f92fc6"
                      "ddf18c5baa3d66303991213d7964b4d4c81dbb9046a777cf93d39263645a"
                      "2f90743735045b9df9c4c0ced169f8"));
  assert(pqcore::shake256(long_message, 200) ==
         hex_to_bytes("e49647491c9d12d125a2f75826c96f6307d2fabebcbb9fb1616d76b09499380"
                      "e8bcf60f72750879140e73fb7453a979b69d25efa8de613462f108ce7f2f1"
                      "d7c5e444637301336604f42850beddef9434234ccc7d84196841069a710537"
                      "9ca1e5c6f79db0e8a7ef1f1ac2f55a76c5c355ddcd4cbac02037a93e18b"
                      "0091df839a02a53df3e5af7a2811b70369652d13019887159d3fc9e8d36"
                      "f0691168b3c7ec1d88a1297c11c020ffa64166889651fcb8cc9e31709737"
                      "01d8cf46faee26a9f8bae301ba265a442bff"));

  assert(pqcore::shake128(abc, 16) != pqcore::shake256(abc, 16));
}
