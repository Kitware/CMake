unset(ENV{MSYSTEM})
cmake_host_system_information(RESULT result QUERY MSYSTEM_PREFIX)
message(STATUS "MSYSTEM_PREFIX='${result}'")
