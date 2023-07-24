cmake_policy(SET CMP0153 OLD)
exec_program("${CMAKE_COMMAND}" ARGS "-E echo \"exec_program() called\"")
