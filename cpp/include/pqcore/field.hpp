#pragma once

#include <cstdint>
#include <ostream>

namespace pqcore {

template <std::int32_t Q>
class ModQ {
  static_assert(Q > 0, "modulus must be positive");

 public:
  constexpr ModQ() = default;
  constexpr explicit ModQ(std::int64_t value) : value_(normalize(value)) {}

  [[nodiscard]] constexpr std::int32_t value() const noexcept { return value_; }

  [[nodiscard]] constexpr std::int32_t centered() const noexcept {
    return value_ > Q / 2 ? value_ - Q : value_;
  }

  constexpr ModQ& operator+=(ModQ rhs) noexcept {
    value_ = normalize(static_cast<std::int64_t>(value_) + rhs.value_);
    return *this;
  }

  constexpr ModQ& operator-=(ModQ rhs) noexcept {
    value_ = normalize(static_cast<std::int64_t>(value_) - rhs.value_);
    return *this;
  }

  friend constexpr ModQ operator+(ModQ lhs, ModQ rhs) noexcept {
    lhs += rhs;
    return lhs;
  }

  friend constexpr ModQ operator-(ModQ lhs, ModQ rhs) noexcept {
    lhs -= rhs;
    return lhs;
  }

  friend constexpr ModQ operator*(ModQ lhs, ModQ rhs) noexcept {
    return ModQ{static_cast<std::int64_t>(lhs.value_) * rhs.value_};
  }

  friend constexpr ModQ operator-(ModQ value) noexcept {
    return ModQ{-static_cast<std::int64_t>(value.value_)};
  }

  friend constexpr bool operator==(ModQ lhs, ModQ rhs) noexcept = default;

 private:
  static constexpr std::int32_t normalize(std::int64_t value) noexcept {
    auto reduced = value % Q;
    if (reduced < 0) {
      reduced += Q;
    }
    return static_cast<std::int32_t>(reduced);
  }

  std::int32_t value_{0};
};

template <std::int32_t Q>
std::ostream& operator<<(std::ostream& stream, ModQ<Q> value) {
  return stream << value.value();
}

}  // namespace pqcore

