cmake_minimum_required(VERSION 3.24)
project(cxx_modules_install_bmi_and_interfaces CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(install_bmi_and_interfaces STATIC)
target_sources(install_bmi_and_interfaces
  PUBLIC
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx)
target_compile_features(install_bmi_and_interfaces PUBLIC cxx_std_20)

install(TARGETS install_bmi_and_interfaces
  ARCHIVE DESTINATION "lib"
  CXX_MODULES_BMI DESTINATION "lib/cxx/bmi"
  FILE_SET CXX_MODULES DESTINATION "lib/cxx/miu")

add_test(NAME check-for-bmi
  COMMAND
    "${CMAKE_COMMAND}"
      "-Dprefix=${CMAKE_INSTALL_PREFIX}"
      "-Dbmi_destination=lib/cxx/bmi"
      "-Dfs_destination=lib/cxx/miu"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/check-for-bmi.cmake")
