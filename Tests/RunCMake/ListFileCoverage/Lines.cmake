# comment
set(a 1)

set(b
  2)
if(a EQUAL 2)
  message(STATUS "two")
elseif(a EQUAL 3)
  message(STATUS "three")
elseif(a EQUAL 1)
  message(STATUS "one")
elseif(a EQUAL 4)
  message(STATUS "four")
ELSE()
  message(STATUS "other")
ENDIF()
foreach(i 1 2 3)
  math(EXPR b "${b} + ${i}")
endforeach()
#[[ bracket
comment ]]
