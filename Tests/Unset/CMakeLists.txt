cmake_minimum_required(VERSION 2.6)
project(Unset C)

# Local variable
set(x 42)
if(NOT x EQUAL 42)
  message(FATAL_ERROR "x!=42")
endif()

if(NOT DEFINED x)
  message(FATAL_ERROR "x should be defined!")
endif()

unset(x)
if(DEFINED x)
  message(FATAL_ERROR "x should be undefined now!")
endif()

# Local variable test unset via set()
set(x 43)
if(NOT x EQUAL 43)
  message(FATAL_ERROR "x!=43")
endif()
set(x)
if(DEFINED x)
  message(FATAL_ERROR "x should be undefined now!")
endif()

# Cache variable
set(BAR "test" CACHE STRING "documentation")
if(NOT DEFINED BAR)
  message(FATAL_ERROR "BAR not defined")
endif()

# Test interaction of cache entries with variables.
set(BAR "test-var")
if(NOT "$CACHE{BAR}" STREQUAL "test")
  message(FATAL_ERROR "\$CACHE{BAR} changed by variable BAR")
endif()
if(NOT "${BAR}" STREQUAL "test-var")
  message(FATAL_ERROR "\${BAR} not separate from \$CACHE{BAR}")
endif()
unset(BAR)
if(NOT "${BAR}" STREQUAL "test")
  message(FATAL_ERROR "\${BAR} does not fall through to \$CACHE{BAR}")
endif()

# Test unsetting of CACHE entry.
unset(BAR CACHE)
if(DEFINED BAR)
  message(FATAL_ERROR "BAR still defined")
endif()


add_executable(Unset unset.c)
