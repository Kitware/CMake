include(RunCMake)

run_cmake_script(no-perms)
run_cmake_script(missing-perms)
run_cmake_script(missing-file-perms)
run_cmake_script(missing-dir-perms)
run_cmake_script(all-perms)
run_cmake_script(invalid-perms)
run_cmake_script(invalid-path)
run_cmake_script(ok)
run_cmake_script(override)

if(UNIX)
  execute_process(COMMAND id -u $ENV{USER}
    OUTPUT_VARIABLE uid
    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if(NOT WIN32 AND NOT MSYS AND NOT "${uid}" STREQUAL "0")
  run_cmake_script(write-only)
endif()
