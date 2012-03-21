set(LOC_FOO TRUE)

set(LotsOfComponents_AComp_FOUND TRUE)
set(LotsOfComponents_BComp_FOUND FALSE)
set(LotsOfComponents_CComp_FOUND TRUE)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(LotsOfComponents REQUIRED_VARS LOC_FOO
                                                   HANDLE_COMPONENTS)
