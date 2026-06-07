#include <cassert>
#include <type_traits>

#include "pqcore/params.hpp"

int main() {
  // Expected values are derived in docs/parameter-tables.md from FIPS 203 Tables 2-3
  // and FIPS 204 Tables 1-2.
  static_assert(!std::is_same_v<pqcore::MlKemParameterSet, pqcore::MlDsaParameterSet>);
  static_assert(std::is_same_v<
                decltype(pqcore::params(pqcore::MlKemParameterSet::MlKem512)),
                pqcore::MlKemParams>);
  static_assert(std::is_same_v<
                decltype(pqcore::params(pqcore::MlDsaParameterSet::MlDsa44)),
                pqcore::MlDsaParams>);

  const auto kem512 = pqcore::params(pqcore::MlKemParameterSet::MlKem512);
  assert(kem512.name == "ML-KEM-512");
  assert(kem512.polynomial_degree == 256);
  assert(kem512.k == 2);
  assert(kem512.q == 3329);
  assert(kem512.eta1 == 3);
  assert(kem512.eta2 == 2);
  assert(kem512.du == 10);
  assert(kem512.dv == 4);
  assert(kem512.rbg_strength_bits == 128);
  assert(kem512.security_category == 1);
  assert(kem512.public_key_bytes == 800);
  assert(kem512.secret_key_bytes == 1632);
  assert(kem512.ciphertext_bytes == 768);
  assert(kem512.shared_secret_bytes == 32);

  const auto kem768 = pqcore::params(pqcore::MlKemParameterSet::MlKem768);
  assert(kem768.name == "ML-KEM-768");
  assert(kem768.polynomial_degree == 256);
  assert(kem768.k == 3);
  assert(kem768.q == 3329);
  assert(kem768.eta1 == 2);
  assert(kem768.eta2 == 2);
  assert(kem768.du == 10);
  assert(kem768.dv == 4);
  assert(kem768.rbg_strength_bits == 192);
  assert(kem768.security_category == 3);
  assert(kem768.public_key_bytes == 1184);
  assert(kem768.secret_key_bytes == 2400);
  assert(kem768.ciphertext_bytes == 1088);
  assert(kem768.shared_secret_bytes == 32);

  const auto kem1024 = pqcore::params(pqcore::MlKemParameterSet::MlKem1024);
  assert(kem1024.name == "ML-KEM-1024");
  assert(kem1024.polynomial_degree == 256);
  assert(kem1024.k == 4);
  assert(kem1024.q == 3329);
  assert(kem1024.eta1 == 2);
  assert(kem1024.eta2 == 2);
  assert(kem1024.du == 11);
  assert(kem1024.dv == 5);
  assert(kem1024.rbg_strength_bits == 256);
  assert(kem1024.security_category == 5);
  assert(kem1024.public_key_bytes == 1568);
  assert(kem1024.secret_key_bytes == 3168);
  assert(kem1024.ciphertext_bytes == 1568);
  assert(kem1024.shared_secret_bytes == 32);

  const auto dsa44 = pqcore::params(pqcore::MlDsaParameterSet::MlDsa44);
  assert(dsa44.name == "ML-DSA-44");
  assert(dsa44.polynomial_degree == 256);
  assert(dsa44.k == 4);
  assert(dsa44.l == 4);
  assert(dsa44.q == 8380417);
  assert(dsa44.zeta == 1753);
  assert(dsa44.d == 13);
  assert(dsa44.tau == 39);
  assert(dsa44.lambda_bits == 128);
  assert(dsa44.gamma1 == (1 << 17));
  assert(dsa44.gamma2 == (8380417 - 1) / 88);
  assert(dsa44.eta == 2);
  assert(dsa44.beta == 78);
  assert(dsa44.omega == 80);
  assert(dsa44.challenge_entropy_bits == 192);
  assert(dsa44.security_category == 2);
  assert(dsa44.public_key_bytes == 1312);
  assert(dsa44.secret_key_bytes == 2560);
  assert(dsa44.signature_bytes == 2420);

  const auto dsa65 = pqcore::params(pqcore::MlDsaParameterSet::MlDsa65);
  assert(dsa65.name == "ML-DSA-65");
  assert(dsa65.polynomial_degree == 256);
  assert(dsa65.k == 6);
  assert(dsa65.l == 5);
  assert(dsa65.q == 8380417);
  assert(dsa65.zeta == 1753);
  assert(dsa65.d == 13);
  assert(dsa65.tau == 49);
  assert(dsa65.lambda_bits == 192);
  assert(dsa65.gamma1 == (1 << 19));
  assert(dsa65.gamma2 == (8380417 - 1) / 32);
  assert(dsa65.eta == 4);
  assert(dsa65.beta == 196);
  assert(dsa65.omega == 55);
  assert(dsa65.challenge_entropy_bits == 225);
  assert(dsa65.security_category == 3);
  assert(dsa65.public_key_bytes == 1952);
  assert(dsa65.secret_key_bytes == 4032);
  assert(dsa65.signature_bytes == 3309);

  const auto dsa87 = pqcore::params(pqcore::MlDsaParameterSet::MlDsa87);
  assert(dsa87.name == "ML-DSA-87");
  assert(dsa87.polynomial_degree == 256);
  assert(dsa87.k == 8);
  assert(dsa87.l == 7);
  assert(dsa87.q == 8380417);
  assert(dsa87.zeta == 1753);
  assert(dsa87.d == 13);
  assert(dsa87.tau == 60);
  assert(dsa87.lambda_bits == 256);
  assert(dsa87.gamma1 == (1 << 19));
  assert(dsa87.gamma2 == (8380417 - 1) / 32);
  assert(dsa87.eta == 2);
  assert(dsa87.beta == 120);
  assert(dsa87.omega == 75);
  assert(dsa87.challenge_entropy_bits == 257);
  assert(dsa87.security_category == 5);
  assert(dsa87.public_key_bytes == 2592);
  assert(dsa87.secret_key_bytes == 4896);
  assert(dsa87.signature_bytes == 4627);
}
