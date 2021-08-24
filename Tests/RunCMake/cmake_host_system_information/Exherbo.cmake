cmake_host_system_information(RESULT TEST1 QUERY DISTRIB_INFO)

foreach(VAR IN LISTS TEST1)
  message(STATUS "${VAR}=`${${VAR}}`")
endforeach()

# Query individual variables
cmake_host_system_information(RESULT TEST2 QUERY DISTRIB_ID DISTRIB_VERSION)
list(POP_FRONT TEST2 TEST2_ID TEST2_VERSION)
message(STATUS "TEST2_ID=`${TEST2_ID}`")
message(STATUS "TEST2_VERSION=`${TEST2_VERSION}`")
