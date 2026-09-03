# With CMP0221 OLD, the no-keyword default follows CMAKE_SYSROOT (target).
cmake_policy(SET CMP0221 OLD)

set(CMAKE_SYSROOT "${CMAKE_CURRENT_LIST_DIR}/Sentinel")
cmake_host_system_information(RESULT def QUERY DISTRIB_ID)

if(NOT def STREQUAL "sentineltarget")
  message(FATAL_ERROR "CMP0221 OLD default read '${def}', expected target 'sentineltarget'")
endif()
