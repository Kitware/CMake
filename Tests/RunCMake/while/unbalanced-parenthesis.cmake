set(var_with_paren "(")
set(some_list "")

while(NOT ${var_with_paren} IN_LIST some_list)
  message(STATUS "Never prints")
endwhile()

message(STATUS "Never prints")
