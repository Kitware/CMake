include(RunCMake)

# Isolate from caller's environment.
set(ENV{CMAKE_APPLE_SILICON_PROCESSOR} "")
set(ENV{CMAKE_OSX_ARCHITECTURES} "")

function(run_arch case)
  set(RunCMake_TEST_OPTIONS ${ARGN})
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${case}-build")
  run_cmake(${case})
  unset(RunCMake_TEST_OPTIONS)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build . --config Debug --verbose)
endfunction()

run_arch(default)

if(RunCMake_GENERATOR MATCHES "Makefiles|Ninja")
  run_arch(default-target-arm64 -DCMAKE_C_COMPILER_TARGET=arm64-apple-macosx)
  run_arch(default-target-x86_64 -DCMAKE_C_COMPILER_TARGET=x86_64-apple-macosx)
endif()

run_arch(arm64-var -DCMAKE_APPLE_SILICON_PROCESSOR=arm64)
run_arch(x86_64-var -DCMAKE_APPLE_SILICON_PROCESSOR=x86_64)

set(ENV{CMAKE_APPLE_SILICON_PROCESSOR} "arm64")
run_arch(arm64-env)

set(ENV{CMAKE_APPLE_SILICON_PROCESSOR} "x86_64")
run_arch(x86_64-env)

set(ENV{CMAKE_APPLE_SILICON_PROCESSOR} "")
