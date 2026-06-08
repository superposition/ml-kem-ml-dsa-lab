cmake_minimum_required(VERSION 3.20)

project(pqcore LANGUAGES CXX)

add_library(pqcore INTERFACE)
add_library(pqcore::pqcore ALIAS pqcore)

include(GNUInstallDirs)

target_include_directories(
  pqcore
  INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_compile_features(pqcore INTERFACE cxx_std_20)

install(
  DIRECTORY include/
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
  TARGETS pqcore
  EXPORT pqcoreTargets
)

install(
  EXPORT pqcoreTargets
  NAMESPACE pqcore::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/pqcore"
)

option(PQCORE_BUILD_TESTS "Build pqcore C++ tests" ON)

if(PQCORE_BUILD_TESTS)
  enable_testing()

  add_executable(test_field tests/test_field.cpp)
  target_link_libraries(test_field PRIVATE pqcore)
  add_test(NAME test_field COMMAND test_field)

  add_executable(test_encoding tests/test_encoding.cpp)
  target_link_libraries(test_encoding PRIVATE pqcore)
  add_test(NAME test_encoding COMMAND test_encoding)

  add_executable(test_ntt tests/test_ntt.cpp)
  target_link_libraries(test_ntt PRIVATE pqcore)
  add_test(NAME test_ntt COMMAND test_ntt)

  add_executable(test_sampling tests/test_sampling.cpp)
  target_link_libraries(test_sampling PRIVATE pqcore)
  target_compile_definitions(test_sampling PRIVATE PQCORE_ENABLE_TEST_SAMPLING=1)
  add_test(NAME test_sampling COMMAND test_sampling)

  add_executable(test_sampling_release_policy tests/test_sampling_release_policy.cpp)
  target_link_libraries(test_sampling_release_policy PRIVATE pqcore)
  add_test(NAME test_sampling_release_policy COMMAND test_sampling_release_policy)

  add_executable(test_sha3 tests/test_sha3.cpp)
  target_link_libraries(test_sha3 PRIVATE pqcore)
  add_test(NAME test_sha3 COMMAND test_sha3)

  add_executable(test_crypto_boundaries tests/test_crypto_boundaries.cpp)
  target_link_libraries(test_crypto_boundaries PRIVATE pqcore)
  add_test(NAME test_crypto_boundaries COMMAND test_crypto_boundaries)

  add_executable(test_pke tests/test_pke.cpp)
  target_link_libraries(test_pke PRIVATE pqcore)
  target_compile_definitions(test_pke PRIVATE PQCORE_ENABLE_TEST_SAMPLING=1)
  add_test(NAME test_pke COMMAND test_pke)

  add_executable(test_pke_release_policy tests/test_pke_release_policy.cpp)
  target_link_libraries(test_pke_release_policy PRIVATE pqcore)
  add_test(NAME test_pke_release_policy COMMAND test_pke_release_policy)

  add_executable(test_kem_internal tests/test_kem_internal.cpp)
  target_link_libraries(test_kem_internal PRIVATE pqcore)
  target_compile_definitions(test_kem_internal PRIVATE PQCORE_ENABLE_TEST_SAMPLING=1)
  add_test(NAME test_kem_internal COMMAND test_kem_internal)

  add_executable(test_kem_internal_release_policy tests/test_kem_internal_release_policy.cpp)
  target_link_libraries(test_kem_internal_release_policy PRIVATE pqcore)
  add_test(NAME test_kem_internal_release_policy COMMAND test_kem_internal_release_policy)

  add_executable(test_dsa_helpers tests/test_dsa_helpers.cpp)
  target_link_libraries(test_dsa_helpers PRIVATE pqcore)
  add_test(NAME test_dsa_helpers COMMAND test_dsa_helpers)

  add_executable(test_dsa_internal tests/test_dsa_internal.cpp)
  target_link_libraries(test_dsa_internal PRIVATE pqcore)
  target_compile_definitions(test_dsa_internal PRIVATE PQCORE_ENABLE_TEST_SAMPLING=1)
  add_test(NAME test_dsa_internal COMMAND test_dsa_internal)

  add_executable(test_dsa_internal_release_policy tests/test_dsa_internal_release_policy.cpp)
  target_link_libraries(test_dsa_internal_release_policy PRIVATE pqcore)
  add_test(NAME test_dsa_internal_release_policy COMMAND test_dsa_internal_release_policy)

  add_executable(test_dsa_public_policy tests/test_dsa_public_policy.cpp)
  target_link_libraries(test_dsa_public_policy PRIVATE pqcore)
  add_test(NAME test_dsa_public_policy COMMAND test_dsa_public_policy)

  add_executable(test_vector_harness tests/test_vector_harness.cpp)
  target_link_libraries(test_vector_harness PRIVATE pqcore)
  add_test(NAME test_vector_harness COMMAND test_vector_harness)

  add_executable(test_secret_data tests/test_secret_data.cpp)
  target_link_libraries(test_secret_data PRIVATE pqcore)
  add_test(NAME test_secret_data COMMAND test_secret_data)

  add_executable(test_polynomial tests/test_polynomial.cpp)
  target_link_libraries(test_polynomial PRIVATE pqcore)
  add_test(NAME test_polynomial COMMAND test_polynomial)

  add_executable(test_params tests/test_params.cpp)
  target_link_libraries(test_params PRIVATE pqcore)
  add_test(NAME test_params COMMAND test_params)

  add_executable(test_api_surface tests/test_api_surface.cpp)
  target_link_libraries(test_api_surface PRIVATE pqcore)
  add_test(NAME test_api_surface COMMAND test_api_surface)
endif()
