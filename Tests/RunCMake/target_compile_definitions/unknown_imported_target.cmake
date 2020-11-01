# Test that target_compile_definitions works on UNKNOWN IMPORTED target
add_library(imported UNKNOWN IMPORTED)
target_compile_definitions(imported INTERFACE FOO)

get_target_property(IMPORTED_INTERFACE_CDS imported INTERFACE_COMPILE_DEFINITIONS)

if (NOT FOO IN_LIST IMPORTED_INTERFACE_CDS)
  message(
    FATAL_ERROR "FOO should be in INTERFACE_COMPILE_DEFINITIONS.\n"
    "Actual INTERFACE_COMPILE_DEFINITIONS: " ${IMPORTED_INTERFACE_CDS})
endif()
