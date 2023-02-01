include(RunCMake)

cmake_policy(SET CMP0057 NEW)

function(run_cmake_with_config test)
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Release)
  endif()
  run_cmake(${test})
endfunction()

run_cmake(TARGET_LINKER_IMPORT_FILE-non-valid-target)
run_cmake(TARGET_LINKER_LIBRARY_FILE-non-valid-target)
run_cmake_with_config(TARGET_IMPORT_FILE)
run_cmake_with_config(TARGET_IMPORT_FILE_SUFFIX)

set (Windows_platforms Windows CYGWIN MSYS)
if (NOT CMAKE_HOST_SYSTEM_NAME IN_LIST Windows_platforms)
  run_cmake(TARGET_SONAME_IMPORT_FILE-non-valid-target)
  run_cmake_with_config(TARGET_SONAME_IMPORT_FILE)
endif()
