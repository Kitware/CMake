set(args)
math(EXPR last_arg_index "${CMAKE_ARGC} - 1")
foreach(arg_index RANGE 4 ${last_arg_index})
  list(APPEND args "${CMAKE_ARGV${arg_index}}")
endforeach()

execute_process(
  COMMAND "${CMAKE_ARGV3}" ${args}
  COMMAND_ERROR_IS_FATAL ANY
)
