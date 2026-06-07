#pragma once

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <span>
#include <string_view>

namespace pqcore {

enum class SecretKind {
  ByteString,
  Seed,
  SecretKey,
  SharedSecret,
  SigningRandom,
  Polynomial,
};

[[nodiscard]] constexpr std::string_view secret_kind_name(SecretKind kind) noexcept {
  switch (kind) {
    case SecretKind::ByteString:
      return "byte-string";
    case SecretKind::Seed:
      return "seed";
    case SecretKind::SecretKey:
      return "secret-key";
    case SecretKind::SharedSecret:
      return "shared-secret";
    case SecretKind::SigningRandom:
      return "signing-random";
    case SecretKind::Polynomial:
      return "polynomial";
  }
  return "unknown";
}

class SecretBytesView {
 public:
  constexpr SecretBytesView(std::string_view label,
                            SecretKind kind,
                            std::span<const std::uint8_t> bytes) noexcept
      : label_(label), kind_(kind), bytes_(bytes) {}

  [[nodiscard]] constexpr std::string_view label() const noexcept { return label_; }
  [[nodiscard]] constexpr SecretKind kind() const noexcept { return kind_; }
  [[nodiscard]] constexpr std::size_t size() const noexcept { return bytes_.size(); }

 private:
  std::string_view label_;
  SecretKind kind_;
  std::span<const std::uint8_t> bytes_;
};

[[nodiscard]] constexpr SecretBytesView secret_bytes_view(
    std::string_view label,
    SecretKind kind,
    std::span<const std::uint8_t> bytes) noexcept {
  return SecretBytesView{label, kind, bytes};
}

inline std::ostream& operator<<(std::ostream& stream, SecretBytesView value) {
  return stream << "SecretBytesView{label=" << value.label()
                << ", kind=" << secret_kind_name(value.kind())
                << ", bytes=<redacted>, size=" << value.size() << "}";
}

}  // namespace pqcore
