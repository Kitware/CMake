include(RunCMake)

run_cmake(DoesNotExist)
run_cmake(Missing)
run_cmake(Function)

macro(run_cmake_install case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS ${ARGN})

  run_cmake(${case})
  run_cmake_command(${case}-install ${CMAKE_COMMAND} -P cmake_install.cmake)
  run_cmake_command(${case}-install-component ${CMAKE_COMMAND} -DCOMPONENT=Unspecified -P cmake_install.cmake)

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
  unset(RunCMake_TEST_OPTIONS)
endmacro()

run_cmake_install(CMP0082-WARN)
run_cmake_install(CMP0082-WARN-Nested)
run_cmake_install(CMP0082-WARN-NestedSub)
run_cmake_install(CMP0082-WARN-None)
run_cmake_install(CMP0082-WARN-NoTopInstall)
run_cmake_install(CMP0082-OLD -DCMP0082_VALUE=OLD)
run_cmake_install(CMP0082-NEW -DCMP0082_VALUE=NEW)

set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ExcludeFromAll-build)
set(RunCMake_TEST_NO_CLEAN 1)

file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

run_cmake(ExcludeFromAll)
set(RunCMake-check-file ExcludeFromAll/check.cmake)
run_cmake_command(ExcludeFromAll-build ${CMAKE_COMMAND} --build .)

unset(RunCMake_TEST_BINARY_DIR)
unset(RunCMake_TEST_NO_CLEAN)
