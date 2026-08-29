# Explicit target selection must read the CMAKE_SYSROOT os-release, even when
# the policy default is host.
cmake_policy(SET CMP0221 NEW)

set(CMAKE_SYSROOT "${CMAKE_CURRENT_LIST_DIR}/Sentinel")
cmake_host_system_information(RESULT tgt QUERY FROM_SYSROOT ON DISTRIB_ID)

if(NOT tgt STREQUAL "sentineltarget")
  message(FATAL_ERROR "FROM_SYSROOT ON read '${tgt}', expected target 'sentineltarget'")
endif()
