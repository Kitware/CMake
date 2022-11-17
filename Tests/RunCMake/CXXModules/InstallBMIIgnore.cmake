enable_language(CXX)

add_library(install-bmi-ignore INTERFACE)

install(TARGETS install-bmi-ignore
  CXX_MODULES_BMI
    # An empty destination ignores BMI installation.
    DESTINATION ""
    COMPONENT "bmi")
