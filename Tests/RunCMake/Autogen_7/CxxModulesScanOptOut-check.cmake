# The Ninja generators name the scan output of every scanned source.
if(NOT RunCMake_GENERATOR MATCHES "Ninja")
  return()
endif()

file(GLOB_RECURSE ninjaFiles "${RunCMake_TEST_BINARY_DIR}/*.ninja")
foreach(ninjaFile IN LISTS ninjaFiles)
  file(STRINGS "${ninjaFile}" scans REGEX "mocs_compilation[^ ]*\\.ddi")
  if(scans)
    string(APPEND RunCMake_TEST_FAILED
      "mocs_compilation.cpp is scanned for C++ module dependencies although "
      "the target sets CXX_SCAN_FOR_MODULES to OFF:\n  ${ninjaFile}\n")
  endif()
endforeach()
