include(RunCMake)

function(run_CMP0058 case)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CMP0058-${case}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(CMP0058-${case})
  run_cmake_command(CMP0058-${case}-build ${CMAKE_COMMAND} --build .)
endfunction()

run_CMP0058(OLD-no)
run_CMP0058(OLD-by)
run_CMP0058(WARN-no)
run_CMP0058(WARN-by)
run_CMP0058(NEW-no)
run_CMP0058(NEW-by)

function(run_SubDir)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/SubDir-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(SubDir)
  run_cmake_command(SubDir-build ${CMAKE_COMMAND} --build . --target SubDir/all)
endfunction()
run_SubDir()
