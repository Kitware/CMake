set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

enable_language(CXX)

add_library(install-bmi SHARED)
target_sources(install-bmi
  PUBLIC
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        sources/importable.cxx)
target_compile_features(install-bmi PUBLIC cxx_std_20)

cmake_diagnostic(SET CMD_INSTALL_ABSOLUTE_DESTINATION FATAL_ERROR)

install(TARGETS install-bmi
  CXX_MODULES_BMI
    DESTINATION "/lib/bmi"
    COMPONENT "bmi")

install(TARGETS install-bmi
  CXX_MODULES_BMI
    DESTINATION "/lib/bmi"
    EXCLUDE_FROM_ALL
    COMPONENT "bmi-optional")

install(TARGETS install-bmi
  CXX_MODULES_BMI
    DESTINATION "/lib/bmi"
    CONFIGURATIONS Debug
    COMPONENT "bmi-only-debug")
