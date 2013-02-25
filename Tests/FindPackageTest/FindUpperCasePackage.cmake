set(UCP_FOO TRUE)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(UpperCasePackage REQUIRED_VARS UCP_FOO
                                                   FOUND_VAR UPPERCASEPACKAGE_FOUND )
