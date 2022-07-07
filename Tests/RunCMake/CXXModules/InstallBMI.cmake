enable_language(CXX)

add_library(install-bmi)
target_sources(install-bmi
  PRIVATE
    sources/cxx-anchor.cxx)

install(TARGETS install-bmi
  CXX_MODULES_BMI
    DESTINATION "lib/bmi"
    COMPONENT "bmi")

install(TARGETS install-bmi
  CXX_MODULES_BMI
    DESTINATION "lib/bmi"
    EXCLUDE_FROM_ALL
    COMPONENT "bmi-optional")

install(TARGETS install-bmi
  CXX_MODULES_BMI
    DESTINATION "lib/bmi"
    CONFIGURATIONS Debug
    COMPONENT "bmi-only-debug")
