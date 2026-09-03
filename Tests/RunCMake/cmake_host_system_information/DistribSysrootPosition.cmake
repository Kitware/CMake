# FROM_SYSROOT is accepted after and between keys, not just as the first token.
cmake_policy(SET CMP0221 NEW)

set(CMAKE_SYSROOT "${CMAKE_CURRENT_LIST_DIR}/Sentinel")

# After a key.
cmake_host_system_information(RESULT after QUERY DISTRIB_ID FROM_SYSROOT ON)
if(NOT after STREQUAL "sentineltarget")
  message(FATAL_ERROR "FROM_SYSROOT after a key read '${after}', expected 'sentineltarget'")
endif()

# Between keys.
cmake_host_system_information(RESULT between QUERY DISTRIB_ID FROM_SYSROOT ON DISTRIB_NAME)
list(GET between 0 between_id)
if(NOT between_id STREQUAL "sentineltarget")
  message(FATAL_ERROR "FROM_SYSROOT between keys read '${between_id}', expected 'sentineltarget'")
endif()
