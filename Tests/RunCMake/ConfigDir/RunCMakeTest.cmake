include(RunCMake)

set(ENV{CMAKE_CONFIG_DIR} ${CMAKE_CURRENT_LIST_DIR}/config)
set(RunCMake-check-file check-reply.cmake)
run_cmake(config)
unset(RunCMake-check-file)
