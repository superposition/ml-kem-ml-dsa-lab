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
