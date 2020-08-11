# pseudo find_module

if (UseComponents_REQUIRE_VARS)
  set(FOOBAR TRUE)
  set(REQUIRED_VARS REQUIRED_VARS  FOOBAR)
endif()

set (UseComponents_Comp1_FOUND TRUE)
set (UseComponents_Comp2_FOUND TRUE)
set (UseComponents_Comp3_FOUND FALSE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UseComponents ${REQUIRED_VARS}
                                                VERSION_VAR Pseudo_VERSION
                                                HANDLE_COMPONENTS)
