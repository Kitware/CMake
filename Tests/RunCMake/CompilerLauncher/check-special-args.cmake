# Verifies that the launcher's own arguments arrive intact, then runs the
# launched command.

set(base 4) # CMAKE_ARGV0..3 are: cmake -P <script> --
set(num_args 4) # <phase> plus the three arguments below

set(expect_1 "")
set(expect_2 "semi;colon")
set(expect_3 "with space")

math(EXPR min_argc "${base} + ${num_args} + 1")
if(CMAKE_ARGC LESS min_argc)
  message(FATAL_ERROR "too few arguments: ${CMAKE_ARGC}")
endif()

foreach(i RANGE 1 3)
  math(EXPR idx "${base} + ${i}")
  if(NOT "${CMAKE_ARGV${idx}}" STREQUAL "${expect_${i}}")
    message(FATAL_ERROR
      "launcher argument ${i}: expected [${expect_${i}}] but got [${CMAKE_ARGV${idx}}]")
  endif()
endforeach()
message("special-args launcher ok: ${CMAKE_ARGV${base}}")

# Build and invoke the launched command via cmake_language(EVAL CODE) using
# bracket arguments, rather than a ";"-separated list. A list element ending
# in a backslash (e.g. MSVC's "/Fd<dir>\") would otherwise merge with the
# list's own separator when the list is later expanded, corrupting the
# command line.
set(cmd_code "execute_process(COMMAND")
math(EXPR first "${base} + ${num_args}")
math(EXPR last "${CMAKE_ARGC} - 1")
foreach(i RANGE ${first} ${last})
  string(APPEND cmd_code " [==[${CMAKE_ARGV${i}}]==]")
endforeach()
string(APPEND cmd_code " RESULT_VARIABLE result)")
cmake_language(EVAL CODE "${cmd_code}")

if(NOT result EQUAL 0)
  message(FATAL_ERROR "launched command failed: ${result}")
endif()
