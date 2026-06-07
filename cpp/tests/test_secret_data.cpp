#include <cassert>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "pqcore/secret_data.hpp"

template <typename T>
concept StreamWritable = requires(std::ostream& stream, const T& value) {
  stream << value;
};

bool contains(const std::string& haystack, const std::string& needle) {
  return haystack.find(needle) != std::string::npos;
}

int main() {
  static_assert(StreamWritable<pqcore::SecretBytesView>);

  const std::vector<std::uint8_t> bytes{0xab, 0xcd, 0xef};
  const auto view = pqcore::secret_bytes_view("side-channel-note",
                                             pqcore::SecretKind::SecretKey,
                                             bytes);

  std::ostringstream stream;
  stream << view;
  const auto text = stream.str();

  assert(contains(text, "side-channel-note"));
  assert(contains(text, "secret-key"));
  assert(contains(text, "<redacted>"));
  assert(contains(text, "size=3"));

  assert(!contains(text, "171"));
  assert(!contains(text, "205"));
  assert(!contains(text, "239"));
  assert(!contains(text, "0xab"));
  assert(!contains(text, "0xcd"));
  assert(!contains(text, "0xef"));
  assert(!contains(text, "ab cd ef"));
}
