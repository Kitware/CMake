include(RunCMake)


function(run_RuntimePath name)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${name}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(${name})
  run_cmake_command(${name}-build ${CMAKE_COMMAND} --build . --config Debug)
endfunction()

run_RuntimePath(SymlinkImplicit)
run_cmake_command(SymlinkImplicitCheck
  ${CMAKE_COMMAND} -Ddir=${RunCMake_BINARY_DIR}/SymlinkImplicit-build -P ${RunCMake_SOURCE_DIR}/SymlinkImplicitCheck.cmake)

run_RuntimePath(Relative)
# FIXME: Run RelativeCheck (appears to be broken currently)

run_RuntimePath(Genex)
run_cmake_command(GenexCheck
  ${CMAKE_COMMAND} -Ddir=${RunCMake_BINARY_DIR}/Genex-build -P ${RunCMake_SOURCE_DIR}/GenexCheck.cmake)
