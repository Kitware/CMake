
set(VAR1 "OUTER1")
set(VAR2 "OUTER2")
set(VAR3 "OUTER3")

while (TRUE)
  # create a block with a new scope for variables
  block(SCOPE_FOR VARIABLES PROPAGATE VAR3)
    set(VAR2 "INNER2" PARENT_SCOPE)
    set(VAR3 "INNER3")
    break()
  endblock()
endwhile()

# check final values for variables
if(NOT DEFINED VAR1 OR NOT VAR1 STREQUAL "OUTER1")
  message(SEND_ERROR "block/endblock: VAR1 has unexpected value: ${VAR1}")
endif()
if(NOT DEFINED VAR2 OR NOT VAR2 STREQUAL "INNER2")
  message(SEND_ERROR "block/endblock: VAR2 has unexpected value: ${VAR2}")
endif()
if(NOT DEFINED VAR3 OR NOT VAR3 STREQUAL "INNER3")
  message(SEND_ERROR "block/endblock: VAR3 has unexpected value: ${VAR3}")
endif()



set(VAR1 "OUTER1")
set(VAR2 "OUTER2")
set(VAR3 "OUTER3")

function (OUTER)
  # create a block with a new scope for variables
  block(SCOPE_FOR VARIABLES PROPAGATE VAR3)
    set(VAR2 "INNER2" PARENT_SCOPE)
    set(VAR3 "INNER3")
    return()
  endblock()
  set(VAR1 "INNER1" PARENT_SCOPE)
endfunction()
outer()

# check final values for variables
if(NOT DEFINED VAR1 OR NOT VAR1 STREQUAL "OUTER1")
  message(SEND_ERROR "block/endblock: VAR1 has unexpected value: ${VAR1}")
endif()
if(NOT DEFINED VAR2 OR NOT VAR2 STREQUAL "OUTER2")
  message(SEND_ERROR "block/endblock: VAR2 has unexpected value: ${VAR2}")
endif()
if(NOT DEFINED VAR3 OR NOT VAR3 STREQUAL "OUTER3")
  message(SEND_ERROR "block/endblock: VAR3 has unexpected value: ${VAR3}")
endif()



set(VAR1 "OUTER1")
set(VAR2 "OUTER2")
set(VAR3 "OUTER3")

foreach (id IN ITEMS 1 2 3)
  # create a block with a new scope for variables
  block(SCOPE_FOR VARIABLES PROPAGATE VAR${id})
    set(VAR${id} "INNER${id}")
    continue()
    set(VAR${id} "BAD${id}")
  endblock()
endforeach()

# check final values for variables
if(NOT DEFINED VAR1 OR NOT VAR1 STREQUAL "INNER1")
  message(SEND_ERROR "block/endblock: VAR1 has unexpected value: ${VAR1}")
endif()
if(NOT DEFINED VAR2 OR NOT VAR2 STREQUAL "INNER2")
  message(SEND_ERROR "block/endblock: VAR2 has unexpected value: ${VAR2}")
endif()
if(NOT DEFINED VAR3 OR NOT VAR3 STREQUAL "INNER3")
  message(SEND_ERROR "block/endblock: VAR3 has unexpected value: ${VAR3}")
endif()
