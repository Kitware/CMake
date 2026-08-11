set(BUILD_DIR "${RunCMake_BINARY_DIR}/OutputSyncUsesTerminal-build")

function(check_has target regex)
  file(STRINGS "${BUILD_DIR}/${target}" lines REGEX "${regex}")
  list(LENGTH lines len)
  if(len EQUAL 0)
    set(RunCMake_TEST_FAILED
      "${RunCMake_TEST_FAILED}Expected to find '${regex}' in ${target}\n"
      PARENT_SCOPE)
  endif()
endfunction()

function(check_missing target regex)
  file(STRINGS "${BUILD_DIR}/${target}" lines REGEX "${regex}")
  list(LENGTH lines len)
  if(NOT len EQUAL 0)
    set(RunCMake_TEST_FAILED
      "${RunCMake_TEST_FAILED}Did not expect to find '${regex}' in ${target}: ${lines}\n"
      PARENT_SCOPE)
  endif()
endfunction()

# The USES_TERMINAL recipe is prefixed with the build-time toggle variable.
check_has("CMakeFiles/term.dir/build.make"
  [[\$\(CMAKE_USES_TERMINAL_PREFIX\)\$\(CMAKE_COMMAND\) -E true]])

# A plain recipe references no prefix.
check_missing("CMakeFiles/plain.dir/build.make" [[\$\(CMAKE_USES_TERMINAL_PREFIX\)]])

# A jobserver-aware recipe keeps the literal '+' and is not given the toggle.
check_has("CMakeFiles/jsa.dir/build.make" [[\+\$\(CMAKE_COMMAND\) -E true]])
check_missing("CMakeFiles/jsa.dir/build.make" [[\$\(CMAKE_USES_TERMINAL_PREFIX\)]])

# A recipe that is both jobserver-aware and USES_TERMINAL uses only '+'
# (no doubled prefix).
check_has("CMakeFiles/both.dir/build.make" [[\+\$\(CMAKE_COMMAND\) -E true]])
check_missing("CMakeFiles/both.dir/build.make" [[\$\(CMAKE_USES_TERMINAL_PREFIX\)]])
