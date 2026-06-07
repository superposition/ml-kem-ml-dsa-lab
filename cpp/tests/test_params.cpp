#include <cassert>

#include "pqcore/params.hpp"

int main() {
  const auto kem768 = pqcore::params(pqcore::MlKemParameterSet::MlKem768);
  assert(kem768.name == "ML-KEM-768");
  assert(kem768.k == 3);
  assert(kem768.q == 3329);
  assert(kem768.public_key_bytes == 1184);
  assert(kem768.ciphertext_bytes == 1088);

  const auto dsa65 = pqcore::params(pqcore::MlDsaParameterSet::MlDsa65);
  assert(dsa65.name == "ML-DSA-65");
  assert(dsa65.k == 6);
  assert(dsa65.l == 5);
  assert(dsa65.q == 8380417);
  assert(dsa65.public_key_bytes == 1952);
  assert(dsa65.secret_key_bytes == 4032);
  assert(dsa65.signature_bytes == 3309);
}

