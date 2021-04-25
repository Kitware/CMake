
cmake_policy(SET CMP0124 OLD)

foreach(VAR a b c)
endforeach()
if (NOT DEFINED VAR OR NOT VAR STREQUAL "")
  message(SEND_ERROR "Variable 'VAR' not defined or wrong value.")
endif()

set(LIST1 a b c)
set(LIST2 x y z)
foreach(VAR1_1 VAR1_2 IN ZIP_LISTS LIST1 LIST2)
endforeach()
if (NOT DEFINED VAR1_1 OR NOT VAR1_1 STREQUAL ""
    OR NOT DEFINED VAR1_2 OR NOT VAR1_2 STREQUAL "")
  message(SEND_ERROR "Variables 'VAR1_1' or 'VAR1_2' not defined or wrong value.")
endif()


set (VAR2 OLD)
foreach(VAR2 a b c)
endforeach()
if (NOT DEFINED VAR2 OR NOT VAR2 STREQUAL "OLD")
  message(SEND_ERROR "Variable 'VAR2' not defined or wrong value.")
endif()

set (VAR2_2 OLD)
foreach(VAR2_1 VAR2_2 IN ZIP_LISTS LIST1 LIST2)
endforeach()
if (NOT DEFINED VAR2_1 OR NOT VAR2_1 STREQUAL ""
    OR NOT DEFINED VAR2_2 OR NOT VAR2_2 STREQUAL "OLD")
  message(SEND_ERROR "Variables 'VAR2_1' or 'VAR2_2' not defined or wrong value.")
endif()


set (VAR3 OLD CACHE STRING "")
foreach(VAR3 a b c)
endforeach()
# a normal variable is defined, holding cache variable value
unset(VAR3 CACHE)
if (NOT DEFINED VAR3 OR NOT VAR3 STREQUAL "OLD")
  message(SEND_ERROR "Variable 'VAR3' not defined or wrong value.")
endif()

set (VAR3_2 OLD CACHE STRING "")
foreach(VAR3_1 VAR3_2 IN ZIP_LISTS LIST1 LIST2)
endforeach()
# a normal variable is defined, holding cache variable value
unset(VAR3_2 CACHE)
if (NOT DEFINED VAR3_1 OR NOT VAR3_1 STREQUAL ""
    OR NOT DEFINED VAR3_2 OR NOT VAR3_2 STREQUAL "OLD")
  message(SEND_ERROR "Variables 'VAR3_1' or 'VAR3_2' not defined or wrong value.")
endif()
