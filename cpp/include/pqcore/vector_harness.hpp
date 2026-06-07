#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "pqcore/dsa.hpp"
#include "pqcore/kem.hpp"
#include "pqcore/params.hpp"

namespace pqcore {

enum class VectorOutcome {
  Passed,
  Pending,
};

struct VectorHarnessCase {
  std::string_view id;
  std::string_view scheme;
  std::string_view operation;
  std::string_view parameter_set;
  std::string_view expected;
};

struct VectorHarnessResult {
  VectorOutcome outcome;
  std::string reason;
};

[[nodiscard]] constexpr std::optional<MlKemParameterSet> ml_kem_parameter_set_from_name(
    std::string_view name) noexcept {
  if (name == "ML-KEM-512") {
    return MlKemParameterSet::MlKem512;
  }
  if (name == "ML-KEM-768") {
    return MlKemParameterSet::MlKem768;
  }
  if (name == "ML-KEM-1024") {
    return MlKemParameterSet::MlKem1024;
  }
  return std::nullopt;
}

[[nodiscard]] constexpr std::optional<MlDsaParameterSet> ml_dsa_parameter_set_from_name(
    std::string_view name) noexcept {
  if (name == "ML-DSA-44") {
    return MlDsaParameterSet::MlDsa44;
  }
  if (name == "ML-DSA-65") {
    return MlDsaParameterSet::MlDsa65;
  }
  if (name == "ML-DSA-87") {
    return MlDsaParameterSet::MlDsa87;
  }
  return std::nullopt;
}

[[nodiscard]] inline VectorHarnessResult run_placeholder_vector_case(
    const VectorHarnessCase& test_case) {
  if (test_case.expected != "not_implemented") {
    throw std::invalid_argument{"placeholder vector case has unsupported expectation"};
  }

  if (test_case.scheme == "ML-KEM") {
    const auto parameter_set = ml_kem_parameter_set_from_name(test_case.parameter_set);
    if (!parameter_set.has_value()) {
      throw std::invalid_argument{"unknown ML-KEM parameter set in vector case"};
    }

    const auto selected = params(*parameter_set);
    try {
      if (test_case.operation == "keyGen") {
        (void)ml_kem_keygen(*parameter_set);
      } else if (test_case.operation == "encap") {
        (void)ml_kem_encapsulate(*parameter_set,
                                 std::vector<std::uint8_t>(selected.public_key_bytes));
      } else if (test_case.operation == "decap") {
        (void)ml_kem_decapsulate(
            *parameter_set,
            std::vector<std::uint8_t>(selected.secret_key_bytes),
            std::vector<std::uint8_t>(selected.ciphertext_bytes));
      } else {
        throw std::invalid_argument{"unknown ML-KEM placeholder vector operation"};
      }
    } catch (const NotImplemented&) {
      return {VectorOutcome::Pending,
              "official ML-KEM vectors are absent; public API remains fail-closed"};
    }
  } else if (test_case.scheme == "ML-DSA") {
    const auto parameter_set = ml_dsa_parameter_set_from_name(test_case.parameter_set);
    if (!parameter_set.has_value()) {
      throw std::invalid_argument{"unknown ML-DSA parameter set in vector case"};
    }

    const auto selected = params(*parameter_set);
    try {
      if (test_case.operation == "keyGen") {
        (void)ml_dsa_keygen(*parameter_set);
      } else if (test_case.operation == "sign") {
        (void)ml_dsa_sign(
            *parameter_set,
            std::vector<std::uint8_t>(selected.secret_key_bytes),
            std::vector<std::uint8_t>{'m'},
            std::vector<std::uint8_t>{});
      } else if (test_case.operation == "verify") {
        (void)ml_dsa_verify(
            *parameter_set,
            std::vector<std::uint8_t>(selected.public_key_bytes),
            std::vector<std::uint8_t>{'m'},
            std::vector<std::uint8_t>(selected.signature_bytes),
            std::vector<std::uint8_t>{});
      } else {
        throw std::invalid_argument{"unknown ML-DSA placeholder vector operation"};
      }
    } catch (const NotImplemented&) {
      return {VectorOutcome::Pending,
              "official ML-DSA vectors are absent; public API remains fail-closed"};
    }
  } else {
    throw std::invalid_argument{"unknown vector case scheme"};
  }

  return {VectorOutcome::Passed, "placeholder vector case executed"};
}

}  // namespace pqcore
