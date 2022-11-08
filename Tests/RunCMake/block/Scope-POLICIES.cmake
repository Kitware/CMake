
set(VAR1 "OUTER1")
set(VAR2 "OUTER2")

set(VARSUB1 "OUTERSUB1")
set(VARSUB2 "OUTERSUB2")

cmake_policy(SET CMP0139 NEW)

# create a block with a new scope for policies
block(SCOPE_FOR POLICIES)
  set(VAR1 "INNER1")
  unset(VAR2)
  set(VAR3 "INNER3")
  add_subdirectory(Scope)

  cmake_policy(SET CMP0139 OLD)
endblock()

# check final values for variables
if(NOT DEFINED VAR1 OR NOT VAR1 STREQUAL "INNER1")
  message(SEND_ERROR "block/endblock: VAR1 has unexpected value: ${VAR1}")
endif()
if(DEFINED VAR2)
  message(SEND_ERROR "block/endblock: VAR2 is unexpectedly defined: ${VAR2}")
endif()
if(NOT DEFINED VAR3 OR NOT VAR3 STREQUAL "INNER3")
  message(SEND_ERROR "block/endblock: VAR3 has unexpected value: ${VAR3}")
endif()
if(NOT DEFINED VARSUB1 OR NOT VARSUB1 STREQUAL "SUBDIR1")
  message(SEND_ERROR "block/endblock: VARSUB1 has unexpected value: ${VARSUB1}")
endif()
if(NOT DEFINED VARSUB2 OR NOT VARSUB2 STREQUAL "SUBDIR2")
  message(SEND_ERROR "block/endblock: VARSUB2 has unexpected value: ${VARSUB2}")
endif()

cmake_policy(GET CMP0139 CMP0139_STATUS)
if(NOT CMP0139_STATUS STREQUAL "NEW")
    message(SEND_ERROR "block/endblock: CMP0139 has unexpected value: ${CMP0139_STATUS}")
endif()
