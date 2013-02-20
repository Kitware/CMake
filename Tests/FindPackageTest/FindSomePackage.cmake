set(SOP_FOO TRUE)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SomePackage REQUIRED_VARS SOP_FOO
                                              USE_ORIGINAL_CASE )
