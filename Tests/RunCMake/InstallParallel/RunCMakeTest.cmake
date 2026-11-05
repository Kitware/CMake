include(RunCMake)

function(install_test test)
  cmake_parse_arguments(ARGS "PARALLEL;NINJA;TOUCH_CACHE" "ARGS;COMPONENT" "" ${ARGN})
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-install)
  set(RunCMake_TEST_OPTIONS -DINSTALL_PARALLEL=${ARGS_PARALLEL} -DCMAKE_INSTALL_PREFIX=install)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  if (ARGS_COMPONENT)
    list(APPEND ARGS_ARGS "--component ${ARGS_COMPONENT}")
  endif()
  run_cmake(install)
  set(RunCMake_TEST_NO_CLEAN 1)
  if (ARGS_TOUCH_CACHE)
    run_cmake_command(${test}-sleep ${CMAKE_COMMAND} -E sleep 2)
    run_cmake_command(${test}-touch
      ${CMAKE_COMMAND} -E touch ${RunCMake_TEST_BINARY_DIR}/CMakeFiles/cmake.check_cache)
  endif()
  if (ARGS_NINJA)
    if (ARGS_PARALLEL)
      set(INSTALL_COUNT 5)
    else()
      set(INSTALL_COUNT 1)
    endif()
    set(RunCMake-check-file check-num-installs.cmake)
    run_cmake_command(${test}-install ${CMAKE_COMMAND} -E env --unset=NINJA_STATUS ${CMAKE_COMMAND} --build . --config Debug ${ARGS_ARGS})
    unset(RunCMake-check-file)
  else()
    if (ARGS_COMPONENT)
      if(ARGS_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
        set(INSTALL_MANIFEST "install_manifest_${ARGS_COMPONENT}.txt")
      else()
        string(MD5 COMPONENT_HASH "${ARGS_COMPONENT}")
        set(INSTALL_MANIFEST "install_manifest_${COMPONENT_HASH}.txt")
      endif()
      set(INSTALL_COUNT 0)
    else()
      set(INSTALL_MANIFEST "install_manifest.txt")
      set(INSTALL_COUNT 5)
    endif()
    set(INSTALL_MANIFEST ${RunCMake_TEST_BINARY_DIR}/${INSTALL_MANIFEST})
    set(RunCMake-check-file check-manifest.cmake)
    run_cmake_command(${test}-install ${CMAKE_COMMAND} --install . ${ARGS_ARGS})
    unset(RunCMake-check-file)
  endif()
endfunction()

install_test(parallel PARALLEL ARGS "-j 4")
install_test(parallel-no-space PARALLEL ARGS "-j4")
install_test(no-parallel ARGS "-j 4")
install_test(out-of-date-json TOUCH_CACHE PARALLEL ARGS "-j 4")
install_test(component PARALLEL ARGS "-j 4" COMPONENT "ALPHANUMERIC123")
install_test(component-hash PARALLEL ARGS "-j 4" COMPONENT "@#$")

if(RunCMake_GENERATOR MATCHES "Ninja")
  install_test(ninja-parallel ARGS "-t install/parallel" NINJA PARALLEL)
  install_test(ninja-no-parallel ARGS "-t install" NINJA)
endif()

# Exit-code fidelity: a failing install must report a non-zero exit code.
function(install_fail_test test fixture)
  cmake_parse_arguments(ARG "PARALLEL" "FAIL_MODE" "INSTALL_ARGS" ${ARGN})
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_OPTIONS
    -DINSTALL_PARALLEL=${ARG_PARALLEL}
    -DCMAKE_INSTALL_PREFIX=install)
  if (ARG_FAIL_MODE)
    list(APPEND RunCMake_TEST_OPTIONS -DFAIL_MODE=${ARG_FAIL_MODE})
  endif()
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  run_cmake(${fixture})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${test}
    ${CMAKE_COMMAND} --install . ${ARG_INSTALL_ARGS})
endfunction()

# Parallel: any failing child must make the install exit non-zero.
install_fail_test(parallel-fatal install-fail PARALLEL FAIL_MODE fatal INSTALL_ARGS -j 4)
install_fail_test(parallel-exit install-fail PARALLEL FAIL_MODE exit INSTALL_ARGS -j 4)
# Serial: fail-fast is preserved on a fatal error (regression guard) ...
install_fail_test(serial-fatal install-fail FAIL_MODE fatal)
# ... and a status-only failure of an earlier component is no longer masked
# by a later success (and the later component is not installed).
install_fail_test(serial-mask install-component-mask
  INSTALL_ARGS --component comp_fail --component comp_ok)

# The stale-index guard exercised by out-of-date-json above must be
# recoverable: once CMakeFiles/InstallScripts.json looks older than
# CMakeFiles/cmake.check_cache, a reconfigure rewrites both files in one
# generate step so the install runs in parallel again.  The handler compares
# their modification times at whole-second resolution, so the freshly generated
# index is not spuriously demoted to the serial fallback on filesystems that
# commit the two configure-time writes out of order within a second.
function(reconfigure_parallel_test test)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-install)
  set(RunCMake_TEST_OPTIONS -DINSTALL_PARALLEL=ON -DCMAKE_INSTALL_PREFIX=install)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  run_cmake(install)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${test}-sleep ${CMAKE_COMMAND} -E sleep 2)
  run_cmake_command(${test}-touch
    ${CMAKE_COMMAND} -E touch ${RunCMake_TEST_BINARY_DIR}/CMakeFiles/cmake.check_cache)
  run_cmake_command(${test}-reconfigure ${CMAKE_COMMAND} .)
  set(INSTALL_MANIFEST ${RunCMake_TEST_BINARY_DIR}/install_manifest.txt)
  set(INSTALL_COUNT 5)
  set(RunCMake-check-file check-manifest.cmake)
  run_cmake_command(${test}-install ${CMAKE_COMMAND} --install .)
  unset(RunCMake-check-file)
endfunction()
reconfigure_parallel_test(out-of-date-json-reconfigure)

# A parallel install does not leave a partial installation behind after a
# missing install script is reported.
function(missing_script_test test)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_OPTIONS -DINSTALL_PARALLEL=ON -DCMAKE_INSTALL_PREFIX=install)
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  run_cmake(install)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE ${RunCMake_TEST_BINARY_DIR}/subdir-1/cmake_install.cmake)
  run_cmake_command(${test} ${CMAKE_COMMAND} --install . -j 4)
endfunction()
missing_script_test(parallel-missing-script)
