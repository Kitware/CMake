enable_language(C)

cmake_policy(SET CMP0185 NEW)

set(Ruby_RBENV_EXECUTABLE "") # Suppress rbenv code path for this test.

find_package(Ruby 1.9.9 REQUIRED)
if (NOT Ruby_FOUND)
  message (FATAL_ERROR "Failed to find Ruby >=1.9.9")
endif()

foreach(var_CMP0185
    RUBY_EXECUTABLE
    RUBY_INCLUDE_DIRS
    RUBY_LIBRARY
    RUBY_VERSION
    )
  if(DEFINED ${var_CMP0185})
    message(FATAL_ERROR "Pre-CMP0185 result variable is set: ${var_CMP0185}")
  endif()
endforeach()
