#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "pqcore/dsa.hpp"

template <typename Exception, typename Fn>
void assert_throws(Fn&& fn) {
  bool threw = false;
  try {
    fn();
  } catch (const Exception&) {
    threw = true;
  }
  assert(threw);
}

int main() {
  const auto selected = pqcore::params(pqcore::MlDsaParameterSet::MlDsa44);
  const std::vector<std::uint8_t> public_key(selected.public_key_bytes);
  const std::vector<std::uint8_t> secret_key(selected.secret_key_bytes);
  const std::vector<std::uint8_t> signature(selected.signature_bytes);
  const std::vector<std::uint8_t> message{'m'};
  const std::vector<std::uint8_t> context{'c'};

  assert_throws<pqcore::NotImplemented>([] {
    (void)pqcore::ml_dsa_keygen(pqcore::MlDsaParameterSet::MlDsa44);
  });

  assert_throws<std::invalid_argument>([&] {
    (void)pqcore::ml_dsa_sign(
        pqcore::MlDsaParameterSet::MlDsa44,
        std::vector<std::uint8_t>(selected.secret_key_bytes - 1),
        message,
        context);
  });
  assert_throws<std::invalid_argument>([&] {
    (void)pqcore::ml_dsa_sign(
        pqcore::MlDsaParameterSet::MlDsa44,
        secret_key,
        message,
        std::vector<std::uint8_t>(256));
  });
  assert_throws<pqcore::NotImplemented>([&] {
    (void)pqcore::ml_dsa_sign(
        pqcore::MlDsaParameterSet::MlDsa44, secret_key, message, context);
  });

  assert_throws<std::invalid_argument>([&] {
    (void)pqcore::ml_dsa_verify(
        pqcore::MlDsaParameterSet::MlDsa44,
        std::vector<std::uint8_t>(selected.public_key_bytes - 1),
        message,
        signature,
        context);
  });
  assert_throws<std::invalid_argument>([&] {
    (void)pqcore::ml_dsa_verify(
        pqcore::MlDsaParameterSet::MlDsa44,
        public_key,
        message,
        std::vector<std::uint8_t>(selected.signature_bytes - 1),
        context);
  });
  assert_throws<std::invalid_argument>([&] {
    (void)pqcore::ml_dsa_verify(
        pqcore::MlDsaParameterSet::MlDsa44,
        public_key,
        message,
        signature,
        std::vector<std::uint8_t>(256));
  });
  assert_throws<pqcore::NotImplemented>([&] {
    (void)pqcore::ml_dsa_verify(
        pqcore::MlDsaParameterSet::MlDsa44, public_key, message, signature, context);
  });
}
