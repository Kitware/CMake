
set(VAR1 "OUTER1")
set(VAR2 "OUTER2")

cmake_policy(SET CMP0139 NEW)

# create a block with a new scope for policies
block(SCOPE_FOR POLICIES)
  set(VAR1 "INNER1")
  unset(VAR2)
  set(VAR3 "INNER3")

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

cmake_policy(GET CMP0139 CMP0139_STATUS)
if(NOT CMP0139_STATUS STREQUAL "NEW")
    message(SEND_ERROR "block/endblock: CMP0139 has unexpected value: ${CMP0139_STATUS}")
endif()
