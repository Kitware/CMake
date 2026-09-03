# With CMP0221 NEW, the no-keyword default ignores CMAKE_SYSROOT (host).
cmake_policy(SET CMP0221 NEW)

cmake_host_system_information(RESULT baseline QUERY DISTRIB_ID)

set(CMAKE_SYSROOT "${CMAKE_CURRENT_LIST_DIR}/Sentinel")
cmake_host_system_information(RESULT def QUERY DISTRIB_ID)

if(NOT def STREQUAL baseline)
  message(FATAL_ERROR "CMP0221 NEW default read '${def}', expected host baseline '${baseline}'")
endif()
if(def STREQUAL "sentineltarget")
  message(FATAL_ERROR "CMP0221 NEW default leaked the target sentinel")
endif()
