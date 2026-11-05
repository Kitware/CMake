include(RunCMake)

set(RunCMake_TEST_OPTIONS "-DFLEX_EXECUTABLE=${FLEX_EXECUTABLE}")

run_cmake(Defaults)
run_cmake(DefinesFile)
run_cmake(Options)
run_cmake(CMP0098-OLD)
run_cmake(BadArgs)
run_cmake(FlexBisonDependency)
run_cmake(FlexBisonDependency-Missing)

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/Build-build)
  run_cmake(Build)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(Build-build ${CMAKE_COMMAND} --build .)
endblock()
