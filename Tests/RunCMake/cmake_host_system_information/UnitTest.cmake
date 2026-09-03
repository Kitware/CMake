cmake_host_system_information(RESULT UNIT_TEST QUERY FROM_SYSROOT ON DISTRIB_INFO)

foreach(VAR IN LISTS UNIT_TEST)
  message(STATUS "${VAR}=`${${VAR}}`")
endforeach()
