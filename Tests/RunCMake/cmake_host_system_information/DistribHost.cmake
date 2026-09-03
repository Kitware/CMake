# Explicit host selection must ignore CMAKE_SYSROOT and read the host os-release.
cmake_policy(SET CMP0221 NEW)

# Baseline: query the host with no sysroot in effect.
cmake_host_system_information(RESULT baseline QUERY DISTRIB_ID)

set(CMAKE_SYSROOT "${CMAKE_CURRENT_LIST_DIR}/Sentinel")
cmake_host_system_information(RESULT host QUERY FROM_SYSROOT OFF DISTRIB_ID)

if(NOT host STREQUAL baseline)
  message(FATAL_ERROR "FROM_SYSROOT OFF read '${host}', expected host baseline '${baseline}'")
endif()
if(host STREQUAL "sentineltarget")
  message(FATAL_ERROR "FROM_SYSROOT OFF leaked the target sentinel")
endif()
