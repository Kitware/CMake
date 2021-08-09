set(var_with_paren "(")
set(some_list "")

if(NOT ${var_with_paren} IN_LIST some_list)
  message(STATUS "Never prints")
else()
  message(STATUS "Never prints")
endif()
