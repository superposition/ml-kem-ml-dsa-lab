#include <cassert>
#include <stdexcept>
#include <vector>

#include "pqcore/dsa.hpp"
#include "pqcore/kem.hpp"

int main() {
  bool saw_not_implemented = false;
  try {
    (void)pqcore::ml_kem_keygen(pqcore::MlKemParameterSet::MlKem512);
  } catch (const pqcore::NotImplemented&) {
    saw_not_implemented = true;
  }
  assert(saw_not_implemented);

  saw_not_implemented = false;
  const auto kem_params = pqcore::params(pqcore::MlKemParameterSet::MlKem512);
  try {
    const std::vector<std::uint8_t> public_key(kem_params.public_key_bytes);
    (void)pqcore::ml_kem_encapsulate(pqcore::MlKemParameterSet::MlKem512, public_key);
  } catch (const pqcore::NotImplemented&) {
    saw_not_implemented = true;
  }
  assert(saw_not_implemented);

  bool saw_invalid_argument = false;
  try {
    const std::vector<std::uint8_t> public_key(1);
    (void)pqcore::ml_kem_encapsulate(pqcore::MlKemParameterSet::MlKem512, public_key);
  } catch (const std::invalid_argument&) {
    saw_invalid_argument = true;
  }
  assert(saw_invalid_argument);

  saw_not_implemented = false;
  const auto dsa_params = pqcore::params(pqcore::MlDsaParameterSet::MlDsa44);
  try {
    const std::vector<std::uint8_t> secret_key(dsa_params.secret_key_bytes);
    const std::vector<std::uint8_t> message{'m'};
    const std::vector<std::uint8_t> context{'c'};
    (void)pqcore::ml_dsa_sign(pqcore::MlDsaParameterSet::MlDsa44, secret_key, message, context);
  } catch (const pqcore::NotImplemented&) {
    saw_not_implemented = true;
  }
  assert(saw_not_implemented);
}

