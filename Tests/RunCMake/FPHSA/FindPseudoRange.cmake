# pseudo find_module

set(FOOBAR TRUE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PseudoRange REQUIRED_VARS FOOBAR VERSION_VAR PseudoRange_VERSION
                                  HANDLE_VERSION_RANGE)
