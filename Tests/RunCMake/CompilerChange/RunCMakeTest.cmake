include(RunCMake)

# Detect the compiler in use in the current environment.
run_cmake(FindCompiler)
include(${RunCMake_BINARY_DIR}/FindCompiler-build/cc.cmake)
if(NOT CMAKE_C_COMPILER)
  message(FATAL_ERROR "FindCompiler provided no compiler!")
endif()
if(NOT IS_ABSOLUTE "${CMAKE_C_COMPILER}")
  message(FATAL_ERROR "FindCompiler provided non-absolute path \"${CMAKE_C_COMPILER}\"!")
endif()
if(NOT EXISTS "${CMAKE_C_COMPILER}")
  message(FATAL_ERROR "FindCompiler provided non-existing path \"${CMAKE_C_COMPILER}\"!")
endif()

# Now that we have the full compiler path, hide CC.
unset(ENV{CC})

# Wrap around the real compiler so we can change the compiler
# path without changing the underlying compiler.
set(ccIn ${RunCMake_SOURCE_DIR}/cc.sh.in)
set(cc1 ${RunCMake_BINARY_DIR}/cc1.sh)
set(cc2 ${RunCMake_BINARY_DIR}/cc2.sh)
set(cc3 CMAKE_C_COMPILER-NOTFOUND)
configure_file(${ccIn} ${cc1} @ONLY)
configure_file(${ccIn} ${cc2} @ONLY)

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ChangeCompiler-build)
  set(ENV{RunCMake_TEST} "FirstCompiler")
  run_cmake_with_options(FirstCompiler -DCMAKE_C_COMPILER=${cc1})
  set(RunCMake_TEST_NO_CLEAN 1)
  set(ENV{RunCMake_TEST} "SecondCompiler")
  run_cmake_with_options(SecondCompiler -DCMAKE_C_COMPILER=${cc2})
  set(ENV{RunCMake_TEST} "EmptyCompiler")
  run_cmake_with_options(EmptyCompiler -DCMAKE_C_COMPILER=)
endblock()

block()
  set(cc1_dot ${RunCMake_BINARY_DIR}/./cc1.sh)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CompilerPath-build)
  set(RunCMake_TEST_VARIANT_DESCRIPTION "-step1")
  run_cmake_with_options(CompilerPath "-DCMAKE_C_COMPILER=${cc1_dot}" -DCACHE_ENTRY=cached)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_VARIANT_DESCRIPTION "-step2")
  run_cmake_with_options(CompilerPath "-DCMAKE_C_COMPILER=${cc1_dot}")
endblock()
