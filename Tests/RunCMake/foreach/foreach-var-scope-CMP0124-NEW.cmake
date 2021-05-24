
cmake_policy(SET CMP0124 NEW)

foreach(VAR a b c)
endforeach()
if (DEFINED VAR)
  message(SEND_ERROR "Variable 'VAR' unexpectedly defined.")
endif()

set(LIST1 a b c)
set(LIST2 x y z)
foreach(VAR1_1 VAR1_2 IN ZIP_LISTS LIST1 LIST2)
endforeach()
if (DEFINED VAR1_1 OR DEFINED VAR1_2)
  message(SEND_ERROR "Variables 'VAR1_1' or 'VAR1_2' unexpectedly defined.")
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
if (DEFINED VAR2_1 OR NOT DEFINED VAR2_2)
  message(SEND_ERROR "Variable 'VAR2_1' unexpectedly defined or variable 'VAR2_2' not defined.")
endif()


set (VAR3 OLD CACHE STRING "")
foreach(VAR3 a b c)
endforeach()
# check that only cache variable is defined
set(OLD_VALUE "${VAR3}")
unset(VAR3 CACHE)
if (DEFINED VAR3 OR NOT OLD_VALUE STREQUAL "OLD")
  message(SEND_ERROR "Variable 'VAR3' wrongly defined or wrong value.")
endif()

set (VAR3_2 OLD CACHE STRING "")
foreach(VAR3_1 VAR3_2 IN ZIP_LISTS LIST1 LIST2)
endforeach()
set(OLD_VALUE "${VAR3_2}")
unset(VAR3_2 CACHE)
if (DEFINED VAR3_1 OR DEFINED VAR3_2 OR NOT OLD_VALUE STREQUAL "OLD")
  message(SEND_ERROR "Variable 'VAR3_1' unexpectedly defined or variable 'VAR2_2' wrongly defined or wrong value.")
endif()
