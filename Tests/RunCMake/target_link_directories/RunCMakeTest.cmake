include(RunCMake)

macro(run_cmake_target test subtest target)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${test}-${subtest} ${CMAKE_COMMAND} --build . --target ${target} ${ARGN})

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
endmacro()

run_cmake(empty_keyword_args)

if(RunCMake_GENERATOR MATCHES "(Ninja|Makefiles)" AND
    NOT RunCMake_GENERATOR MATCHES "(NMake|Borland)")
  set(RunCMake_TEST_OUTPUT_MERGE TRUE)
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Release)
  endif()
  if (RunCMake_GENERATOR MATCHES "Ninja")
    set(VERBOSE -- -v)
  endif()

  run_cmake(CMP0099-NEW)
  run_cmake_target(CMP0099-NEW basic LinkDirs_exe ${VERBOSE})


  run_cmake(CMP0099-OLD)
  run_cmake_target(CMP0099-OLD basic LinkDirs_exe ${VERBOSE})

  unset(RunCMake_TEST_OPTIONS)
  unset(RunCMake_TEST_OUTPUT_MERGE)
endif()
