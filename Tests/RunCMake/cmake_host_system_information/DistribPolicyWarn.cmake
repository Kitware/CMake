# CMP0221 unset (WARN): a DISTRIB_* query with a non-empty CMAKE_SYSROOT and no
# selector keyword warns once and uses the OLD behavior (target).
set(CMAKE_SYSROOT "${CMAKE_CURRENT_LIST_DIR}/Sentinel")
cmake_host_system_information(RESULT def QUERY DISTRIB_ID)

if(NOT def STREQUAL "sentineltarget")
  message(FATAL_ERROR "CMP0221 WARN read '${def}', expected target 'sentineltarget'")
endif()
