function(my_func arg1 arg2)
  message("arg1: ${arg1}, arg2: ${arg2}")
  set(RESULT "${arg1}_${arg2}" PARENT_SCOPE)
endfunction()

my_func("hello" "world")
message("Result: ${RESULT}")
