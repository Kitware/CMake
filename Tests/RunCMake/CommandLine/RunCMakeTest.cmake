include(RunCMake)

run_cmake_command(build-no-cache
  ${CMAKE_COMMAND} --build ${RunCMake_SOURCE_DIR})
run_cmake_command(build-no-generator
  ${CMAKE_COMMAND} --build ${RunCMake_SOURCE_DIR}/cache-no-generator)
run_cmake_command(build-bad-generator
  ${CMAKE_COMMAND} --build ${RunCMake_SOURCE_DIR}/cache-bad-generator)

if(RunCMake_GENERATOR STREQUAL "Ninja")
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/Build-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  set(RunCMake_TEST_OPTIONS -DCMAKE_VERBOSE_MAKEFILE=1)
  run_cmake(Build)
  unset(RunCMake_TEST_OPTIONS)
  run_cmake_command(Build-ninja-v ${CMAKE_COMMAND} --build .)

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
endif()

if(UNIX)
  run_cmake_command(E_create_symlink-missing-dir
    ${CMAKE_COMMAND} -E create_symlink T missing-dir/L
    )

  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR
    ${RunCMake_BINARY_DIR}/E_create_symlink-broken-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  run_cmake_command(E_create_symlink-broken-create
    ${CMAKE_COMMAND} -E create_symlink T L
    )
  run_cmake_command(E_create_symlink-broken-replace
    ${CMAKE_COMMAND} -E create_symlink . L
    )
  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)

  run_cmake_command(E_create_symlink-no-replace-dir
    ${CMAKE_COMMAND} -E create_symlink T .
    )
endif()

run_cmake_command(E_env-no-command0 ${CMAKE_COMMAND} -E env)
run_cmake_command(E_env-no-command1 ${CMAKE_COMMAND} -E env TEST_ENV=1)
run_cmake_command(E_env-bad-arg1 ${CMAKE_COMMAND} -E env -bad-arg1)
run_cmake_command(E_env-set   ${CMAKE_COMMAND} -E env TEST_ENV=1 ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/E_env-set.cmake)
run_cmake_command(E_env-unset ${CMAKE_COMMAND} -E env TEST_ENV=1 ${CMAKE_COMMAND} -E env --unset=TEST_ENV ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/E_env-unset.cmake)

set(RunCMake_DEFAULT_stderr ".")
run_cmake_command(E_sleep-no-args ${CMAKE_COMMAND} -E sleep)
unset(RunCMake_DEFAULT_stderr)
run_cmake_command(E_sleep-bad-arg1 ${CMAKE_COMMAND} -E sleep x)
run_cmake_command(E_sleep-bad-arg2 ${CMAKE_COMMAND} -E sleep 1 -1)
run_cmake_command(E_sleep-one-tenth ${CMAKE_COMMAND} -E sleep 0.1)

run_cmake_command(P_directory ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR})

set(RunCMake_TEST_OPTIONS
  "-DFOO=-DBAR:BOOL=BAZ")
run_cmake(D_nested_cache)

set(RunCMake_TEST_OPTIONS
  "-DFOO:STRING=-DBAR:BOOL=BAZ")
run_cmake(D_typed_nested_cache)

function(run_cmake_depends)
  set(RunCMake_TEST_SOURCE_DIR "${RunCMake_SOURCE_DIR}/cmake_depends")
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/cmake_depends-build")
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/DepTarget.dir/DependInfo.cmake" "
set(CMAKE_DEPENDS_LANGUAGES \"C\")
set(CMAKE_DEPENDS_CHECK_C
  \"${RunCMake_TEST_SOURCE_DIR}/test.c\"
  \"${RunCMake_TEST_BINARY_DIR}/CMakeFiles/DepTarget.dir/test.c.o\"
  )
")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/CMakeDirectoryInformation.cmake" "
set(CMAKE_RELATIVE_PATH_TOP_SOURCE \"${RunCMake_TEST_SOURCE_DIR}\")
set(CMAKE_RELATIVE_PATH_TOP_BINARY \"${RunCMake_TEST_BINARY_DIR}\")
")
  run_cmake_command(cmake_depends ${CMAKE_COMMAND} -E cmake_depends
    "Unix Makefiles"
    ${RunCMake_TEST_SOURCE_DIR} ${RunCMake_TEST_SOURCE_DIR}
    ${RunCMake_TEST_BINARY_DIR} ${RunCMake_TEST_BINARY_DIR}
    ${RunCMake_TEST_BINARY_DIR}/CMakeFiles/DepTarget.dir/DependInfo.cmake
    )
endfunction()
run_cmake_depends()
