# Fortran modules are produced by compiling the dependency, so the
# consumer must keep the strong target-level ordering.
file(READ "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/Makefile2" make2)
string(REPLACE "\\" "/" make2 "${make2}")
string(REPLACE "\n" "\n  " make2 "  ${make2}")
if(NOT make2 MATCHES "CMakeFiles/fmain\\.dir/depend:[^\n]*CMakeFiles/fdep\\.dir/all")
  string(APPEND RunCMake_TEST_FAILED
    "fmain.dir/depend does not depend on fdep.dir/all:\n${make2}\n")
endif()
