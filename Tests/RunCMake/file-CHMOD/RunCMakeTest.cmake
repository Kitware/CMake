include(RunCMake)

run_cmake(CHMOD-no-perms)
run_cmake(CHMOD-no-keyword)
run_cmake(CHMOD-all-perms)
run_cmake(CHMOD-invalid-perms)
run_cmake(CHMOD-invalid-path)
run_cmake(CHMOD-ok)
run_cmake(CHMOD-override)

if(UNIX)
  execute_process(COMMAND id -u $ENV{USER}
    OUTPUT_VARIABLE uid
    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if(NOT WIN32 AND NOT MSYS AND NOT "${uid}" STREQUAL "0")
  run_cmake(CHMOD-write-only)
endif()
