cmake_path(APPEND bin_dir _CPack_Packages/Linux/RPM/SPECS OUTPUT_VARIABLE SPECS)

file(GLOB specs "${SPECS}/*.spec")
if(NOT specs)
  return()
endif()

set(_pfx multilene_defines_test)
set(
    expected_defines
    "%define ${_pfx}_satu 1"
    "%define ${_pfx}_dua 2"
    "%define ${_pfx}_tiga 3"
  )

foreach(spec IN LISTS specs)
  file(STRINGS "${spec}" defines REGEX "%define multilene_defines_test_*.")
  if(NOT defines STREQUAL expected_defines)
    message(
        FATAL_ERROR
        "Multiline definitions mismatch:\n"
        "      file: `${spec}`\n"
        "  expected: `${expected_defines}`\n"
        "    actual: `${defines}`\n"
      )
  endif()
endforeach()
