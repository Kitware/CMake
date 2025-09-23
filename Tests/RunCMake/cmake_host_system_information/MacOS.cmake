set(ENV{MallocGuardEdges} 1) # test tolerating sw_vers stderr
cmake_host_system_information(RESULT os_name QUERY OS_NAME)
message(STATUS "os_name='${os_name}'")
