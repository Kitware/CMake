cmake_host_system_information(RESULT DEBIAN6 QUERY FROM_SYSROOT ON DISTRIB_INFO)

foreach(VAR IN LISTS DEBIAN6)
  message(STATUS "${VAR}=`${${VAR}}`")
endforeach()
