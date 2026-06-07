#include <cassert>
#include <stdexcept>
#include <string>

#include "pqcore/vector_harness.hpp"

template <typename Fn>
void assert_throws_invalid_argument(Fn&& fn) {
  bool threw = false;
  try {
    fn();
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);
}

int main() {
  static_assert(pqcore::ml_kem_parameter_set_from_name("ML-KEM-512").has_value());
  static_assert(pqcore::ml_kem_parameter_set_from_name("ML-KEM-768").has_value());
  static_assert(pqcore::ml_kem_parameter_set_from_name("ML-KEM-1024").has_value());
  static_assert(!pqcore::ml_kem_parameter_set_from_name("Kyber512").has_value());
  static_assert(pqcore::ml_dsa_parameter_set_from_name("ML-DSA-44").has_value());
  static_assert(pqcore::ml_dsa_parameter_set_from_name("ML-DSA-65").has_value());
  static_assert(pqcore::ml_dsa_parameter_set_from_name("ML-DSA-87").has_value());
  static_assert(!pqcore::ml_dsa_parameter_set_from_name("Dilithium2").has_value());

  const auto kem_result = pqcore::run_placeholder_vector_case(
      {"placeholder-kem-keygen", "ML-KEM", "keyGen", "ML-KEM-512", "not_implemented"});
  assert(kem_result.outcome == pqcore::VectorOutcome::Pending);
  assert(kem_result.reason.find("ML-KEM") != std::string::npos);
  assert(kem_result.reason.find("fail-closed") != std::string::npos);

  const auto official_kem_result = pqcore::validate_official_vector_case_metadata({
      "acvp-ml-kem-keygen-fips203-tg1-tc1",
      "ML-KEM",
      "keyGen",
      "ML-KEM-512",
      1,
      1,
      true,
      true,
  });
  assert(official_kem_result.outcome == pqcore::VectorOutcome::Pending);
  assert(official_kem_result.reason.find("official vector") != std::string::npos);
  assert(pqcore::known_official_mode("ML-KEM", "encapDecap"));
  assert(!pqcore::known_official_mode("ML-KEM", "sigVer"));

  const auto dsa_result = pqcore::run_placeholder_vector_case(
      {"placeholder-dsa-sign", "ML-DSA", "sign", "ML-DSA-44", "not_implemented"});
  assert(dsa_result.outcome == pqcore::VectorOutcome::Pending);
  assert(dsa_result.reason.find("ML-DSA") != std::string::npos);
  assert(dsa_result.reason.find("fail-closed") != std::string::npos);

  const auto official_dsa_result = pqcore::validate_official_vector_case_metadata({
      "acvp-ml-dsa-sigver-fips204-tg1-tc1",
      "ML-DSA",
      "sigVer",
      "ML-DSA-44",
      1,
      1,
      true,
      true,
  });
  assert(official_dsa_result.outcome == pqcore::VectorOutcome::Pending);
  assert(official_dsa_result.reason.find("algorithm execution is pending") != std::string::npos);
  assert(pqcore::known_official_mode("ML-DSA", "sigGen"));
  assert(!pqcore::known_official_mode("ML-DSA", "encapDecap"));

  assert_throws_invalid_argument([] {
    (void)pqcore::run_placeholder_vector_case(
        {"bad-expected", "ML-KEM", "keyGen", "ML-KEM-512", "success"});
  });
  assert_throws_invalid_argument([] {
    (void)pqcore::run_placeholder_vector_case(
        {"bad-kem-parameter", "ML-KEM", "keyGen", "Kyber512", "not_implemented"});
  });
  assert_throws_invalid_argument([] {
    (void)pqcore::run_placeholder_vector_case(
        {"bad-dsa-parameter", "ML-DSA", "sign", "Dilithium2", "not_implemented"});
  });
  assert_throws_invalid_argument([] {
    (void)pqcore::run_placeholder_vector_case(
        {"bad-scheme", "SPHINCS+", "sign", "SLH-DSA-SHA2-128s", "not_implemented"});
  });
  assert_throws_invalid_argument([] {
    (void)pqcore::validate_official_vector_case_metadata({
        "bad-official-mode", "ML-KEM", "sigVer", "ML-KEM-512", 1, 1, true, true});
  });
  assert_throws_invalid_argument([] {
    (void)pqcore::validate_official_vector_case_metadata({
        "bad-official-parameter", "ML-DSA", "sigVer", "ML-KEM-512", 1, 1, true, true});
  });
  assert_throws_invalid_argument([] {
    (void)pqcore::validate_official_vector_case_metadata({
        "bad-official-case-id", "ML-DSA", "sigVer", "ML-DSA-44", 1, 0, true, true});
  });
  assert_throws_invalid_argument([] {
    (void)pqcore::validate_official_vector_case_metadata({
        "bad-official-files", "ML-DSA", "sigVer", "ML-DSA-44", 1, 1, true, false});
  });
}
