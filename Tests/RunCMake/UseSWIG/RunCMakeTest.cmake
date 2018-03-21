include(RunCMake)

function(run_SWIG test)
  cmake_parse_arguments(_SWIG_TEST "" "TARGET" "" ${ARGN})
  if (_SWIG_TEST_TARGET)
    list (INSERT _SWIG_TEST_TARGET 0 --target)
  endif()

  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set (RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Release)
  endif()
  run_cmake(${test})
  run_cmake_command(${test}-test ${CMAKE_COMMAND} --build ${RunCMake_TEST_BINARY_DIR} --config Release ${_SWIG_TEST_TARGET})
endfunction()

run_SWIG(LegacyPython TARGET RunTest)
run_SWIG(LegacyPerl TARGET RunTest)

run_SWIG(BasicPython TARGET RunTest)
run_SWIG(BasicPerl TARGET RunTest)

run_SWIG(MultipleModules)
run_SWIG(MultiplePython)
