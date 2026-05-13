include(RunCMake)

function(run_cmake_cache_newline_test case)
  set(RunCMake-check-file CheckCache.cmake)
  run_cmake(${case})

  # Sanity check that the CMakeCache.txt is valid by trying to list variables.
  unset(RunCMake-check-file)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
  run_cmake_command(${case}-List ${CMAKE_COMMAND} -L ${RunCMake_TEST_BINARY_DIR})
endfunction()

run_cmake_cache_newline_test(CacheNewlines)
run_cmake_cache_newline_test(CacheStartsWithNewline)
run_cmake_cache_newline_test(CacheSingleNewline)
