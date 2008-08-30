cmake_minimum_required(VERSION 2.6)
project(Unset C)

# Local variable
set(x 42)
if(NOT x EQUAL 42)
  message(FATAL_ERROR "x!=42")
endif(NOT x EQUAL 42)

if(NOT DEFINED x)
  message(FATAL_ERROR "x should be defined!")
endif(NOT DEFINED x)

unset(x)
if(DEFINED x)
  message(FATAL_ERROR "x should be undefined now!")
endif(DEFINED x)

# Local variable test unset via set()
set(x 43)
if(NOT x EQUAL 43)
  message(FATAL_ERROR "x!=43")
endif(NOT x EQUAL 43)
set(x)
if(DEFINED x)
  message(FATAL_ERROR "x should be undefined now!")
endif(DEFINED x)

# Cache variable
set(BAR "test" CACHE STRING "documentation")
if(NOT DEFINED BAR)
  message(FATAL_ERROR "BAR not defined")
endif(NOT DEFINED BAR)

unset(BAR CACHE)
if(DEFINED BAR)
  message(FATAL_ERROR "BAR still defined")
endif(DEFINED BAR)


add_executable(Unset unset.c)
