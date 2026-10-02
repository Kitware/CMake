cmake_minimum_required(VERSION 4.4)
project(cxx_modules_install_prep_bmi CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(install_bmi STATIC)
target_sources(install_bmi
  PUBLIC
    FILE_SET CXX_MODULES
      BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES importable.cxx)
target_compile_features(install_bmi PUBLIC cxx_std_20)

set(install_targets install_bmi)
set(archive_destination "lib")
if (prep_test)
  set_property(TARGET install_bmi PROPERTY EXCLUDE_FROM_ALL 1)
  add_library(no_modules STATIC ../install-prep-never-build.cxx)
  add_library(unrelated STATIC ../install-prep-never-build.cxx)

  # Private module file sets have no installable BMIs.
  add_library(private_module STATIC EXCLUDE_FROM_ALL)
  target_sources(private_module PRIVATE FILE_SET CXX_MODULES FILES private.cxx)
  target_compile_features(private_module PRIVATE cxx_std_20)

  list(APPEND install_targets no_modules private_module)
  set(archive_destination "artifacts")
  set(archive_options COMPONENT Artifacts EXCLUDE_FROM_ALL)
  set(source_options FILE_SET CXX_MODULES DESTINATION "sources"
    COMPONENT Sources EXCLUDE_FROM_ALL)
endif ()

set(bmi_destination "lib/cxx/bmi")
if (prep_empty_destination)
  set(bmi_destination "$<0:lib/cxx/bmi>")
endif ()
install(TARGETS ${install_targets}
  ARCHIVE DESTINATION "${archive_destination}" ${archive_options}
  ${source_options}
  CXX_MODULES_BMI DESTINATION "${bmi_destination}" COMPONENT BMI ${prep_rule_options})
