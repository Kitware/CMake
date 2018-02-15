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
  run_cmake(${test})
  run_cmake_command(${test}-test ${CMAKE_COMMAND} --build ${RunCMake_TEST_BINARY_DIR} ${_SWIG_TEST_TARGET})
endfunction()

run_SWIG(LegacyPython TARGET RunTest)
run_SWIG(LegacyPerl TARGET RunTest)

run_SWIG(BasicPython TARGET RunTest)
run_SWIG(BasicPerl TARGET RunTest)

run_SWIG(MultipleModules)
