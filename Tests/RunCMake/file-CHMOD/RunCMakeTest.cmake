include(RunCMake)

run_cmake(no-perms)
run_cmake(no-keyword)
run_cmake(all-perms)
run_cmake(invalid-perms)
run_cmake(invalid-path)
run_cmake(ok)
run_cmake(override)

if(UNIX)
  execute_process(COMMAND id -u $ENV{USER}
    OUTPUT_VARIABLE uid
    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if(NOT WIN32 AND NOT MSYS AND NOT "${uid}" STREQUAL "0")
  run_cmake(write-only)
endif()
