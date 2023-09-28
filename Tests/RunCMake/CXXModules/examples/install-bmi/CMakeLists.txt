cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_install_bmi CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(install_bmi STATIC)
target_sources(install_bmi
  PUBLIC
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx)
target_compile_features(install_bmi PUBLIC cxx_std_20)

install(TARGETS install_bmi
  ARCHIVE DESTINATION "lib"
  CXX_MODULES_BMI DESTINATION "lib/cxx/bmi")

add_test(NAME check-for-bmi
  COMMAND
    "${CMAKE_COMMAND}"
      "-Dprefix=${CMAKE_INSTALL_PREFIX}"
      "-Ddestination=lib/cxx/bmi"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/check-for-bmi.cmake")
