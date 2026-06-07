#include <cassert>
#include <cstdint>
#include <type_traits>

#include "pqcore/field.hpp"

int main() {
  static_assert(pqcore::MlKemField::modulus() == 3329);
  static_assert(pqcore::MlDsaField::modulus() == 8380417);
  static_assert(!std::is_same_v<pqcore::MlKemField, pqcore::MlDsaField>);

  assert(pqcore::MlKemField{3329}.value() == 0);
  assert(pqcore::MlKemField{3330}.value() == 1);
  assert(pqcore::MlKemField{-1}.value() == 3328);
  assert(pqcore::MlKemField{-3330}.value() == 3328);

  assert(pqcore::MlDsaField{8380417}.value() == 0);
  assert(pqcore::MlDsaField{8380418}.value() == 1);
  assert(pqcore::MlDsaField{-1}.value() == 8380416);
  assert(pqcore::MlDsaField{-8380418}.value() == 8380416);

  const auto kem_high = pqcore::MlKemField{3328};
  const auto kem_two = pqcore::MlKemField{2};
  assert((kem_high + kem_two).value() == 1);
  assert((kem_two - kem_high).value() == 3);
  assert((kem_high * kem_high).value() == 1);
  assert((-kem_two).value() == 3327);

  const auto dsa_high = pqcore::MlDsaField{8380416};
  const auto dsa_two = pqcore::MlDsaField{2};
  assert((dsa_high + dsa_two).value() == 1);
  assert((dsa_two - dsa_high).value() == 3);
  assert((dsa_high * dsa_high).value() == 1);
  assert((pqcore::MlDsaField{8380415} * pqcore::MlDsaField{8380414}).value() == 6);
  assert((-dsa_two).value() == 8380415);

  assert(pqcore::ModQ<17>{8}.centered() == 8);
  assert(pqcore::ModQ<17>{9}.centered() == -8);
  assert(pqcore::MlKemField{1664}.centered() == 1664);
  assert(pqcore::MlKemField{1665}.centered() == -1664);
  assert(pqcore::MlDsaField{4190208}.centered() == 4190208);
  assert(pqcore::MlDsaField{4190209}.centered() == -4190208);

  for (std::int64_t value = -10000; value <= 10000; ++value) {
    auto expected = value % pqcore::MlKemField::modulus();
    if (expected < 0) {
      expected += pqcore::MlKemField::modulus();
    }
    assert(pqcore::MlKemField{value}.value() == expected);
  }

  for (std::int64_t value = -10000; value <= 10000; ++value) {
    auto expected = value % pqcore::MlDsaField::modulus();
    if (expected < 0) {
      expected += pqcore::MlDsaField::modulus();
    }
    assert(pqcore::MlDsaField{value}.value() == expected);
  }
}
