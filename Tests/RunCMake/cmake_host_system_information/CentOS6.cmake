cmake_host_system_information(RESULT CENTOS6 QUERY DISTRIB_INFO)

foreach(VAR IN LISTS CENTOS6)
  message(STATUS "${VAR}=`${${VAR}}`")
endforeach()
