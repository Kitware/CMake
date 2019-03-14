include(RunCMake)
include(RunCTest)

function(run_cmake_recursive name)
  set(RunCMake_TEST_OPTIONS "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" -DTEST_NAME=${name})
  run_cmake(${name}-default)
  unset(RunCMake_TEST_OPTIONS)
  set(RunCMake_TEST_OPTIONS "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" -DTEST_NAME=${name} -DCMAKE_MAXIMUM_RECURSION_DEPTH=10)
  run_cmake(${name}-var)
  unset(RunCMake_TEST_OPTIONS)
  set(RunCMake_TEST_OPTIONS "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" -DTEST_NAME=${name} -DCMAKE_MAXIMUM_RECURSION_DEPTH=a)
  run_cmake(${name}-invalid-var)
  unset(RunCMake_TEST_OPTIONS)

  run_cmake_command(${name}-default-script ${CMAKE_COMMAND} "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" -DTEST_NAME=${name} -P "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")
  run_cmake_command(${name}-var-script ${CMAKE_COMMAND} "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" -DTEST_NAME=${name} -DCMAKE_MAXIMUM_RECURSION_DEPTH=10 -P "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")
  run_cmake_command(${name}-invalid-var-script ${CMAKE_COMMAND} "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" -DTEST_NAME=${name} -DCMAKE_MAXIMUM_RECURSION_DEPTH=a -P "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")
endfunction()

function(run_ctest_recursive name)
  run_ctest(${name}-default "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" -DTEST_NAME=${name})
  run_ctest(${name}-var "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" -DTEST_NAME=${name} -DCMAKE_MAXIMUM_RECURSION_DEPTH=10)
  run_ctest(${name}-invalid-var "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" -DTEST_NAME=${name} -DCMAKE_MAXIMUM_RECURSION_DEPTH=a)
endfunction()

run_cmake_recursive(function)
run_cmake_recursive(macro)
run_cmake_recursive(include)
run_cmake_recursive(find_package)
run_cmake_recursive(variable_watch)

# We run these tests separately and only with a small limit because they are
# taxing and slow. The "implicit" and "invalid" cases are already thoroughly
# covered by the other tests above.
set(RunCMake_TEST_OPTIONS "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" -DTEST_NAME=add_subdirectory -DCMAKE_MAXIMUM_RECURSION_DEPTH=10)
run_cmake(add_subdirectory-var)
unset(RunCMake_TEST_OPTIONS)
set(RunCMake_TEST_OPTIONS "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" -DTEST_NAME=try_compile -DCMAKE_MAXIMUM_RECURSION_DEPTH=10)
run_cmake(try_compile-var)
unset(RunCMake_TEST_OPTIONS)

run_ctest_recursive(ctest_read_custom_files)

# We run the ctest_run_script() test separately and only with an explicit limit
# because ctest_run_script() is taxing and slow, and because the implicit
# recursion limit is hit by CTestScriptMode.cmake before we can test it
# properly. The "implicit" and "invalid" cases are already thoroughly covered
# by the other tests above.
run_ctest(ctest_run_script-var "-DCMAKE_MODULE_PATH=${CMAKE_CURRENT_LIST_DIR}" -DTEST_NAME=ctest_run_script -DCMAKE_MAXIMUM_RECURSION_DEPTH=10)
