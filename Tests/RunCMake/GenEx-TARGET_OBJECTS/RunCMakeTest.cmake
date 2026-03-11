include(RunCMake)

function(run_cmake_and_build name)
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${name}-build")
  run_cmake_with_options(${name} -DCMake_TEST_CUDA=${CMake_TEST_CUDA})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${name}-build ${CMAKE_COMMAND} --build . --verbose)
endfunction()

run_cmake_and_build(all)
run_cmake_and_build(SOURCE_FILES)

run_cmake(SOURCE_FILES-noexist)
run_cmake(SOURCE_FILES-not-in-target)
run_cmake(SOURCE_FILES-imported)
run_cmake(SOURCE_FILES-relative)

run_cmake(invalid)
