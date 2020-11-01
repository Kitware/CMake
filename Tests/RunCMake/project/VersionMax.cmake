cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0096 OLD)

enable_language(C)
include(CheckTypeSize)
check_type_size(unsigned __sizeOfUnsigned BUILTIN_TYPES_ONLY LANGUAGE C)

# We can't use math() to compute this because it only supports up to
# 64-bit signed integers, so hard-code the types we expect to encounter
if(__sizeOfUnsigned EQUAL 0)
  message(STATUS "Multi-architecture build, skipping project version check")
  return()
elseif(__sizeOfUnsigned EQUAL 4)
  set(maxVal 4294967295)
elseif(__sizeOfUnsigned EQUAL 8)
  set(maxVal 18446744073709551615)
else()
  message(FATAL_ERROR
    "Test needs to be updated for unsigned integer size ${__sizeOfUnsigned}")
endif()

# The real value of this test is when an address sanitizer is enabled.
# It catches situations where the size of the buffer used to compute or
# hold the version components as strings is too small.
project(ProjectA VERSION ${maxVal}.${maxVal}.${maxVal}.${maxVal} LANGUAGES NONE)

if(NOT ${PROJECT_VERSION_MAJOR} EQUAL ${maxVal})
  message(FATAL_ERROR "Project version number parsing failed round trip.\n"
    "Expected: ${maxVal}\n"
    "Computed: ${PROJECT_VERSION_MAJOR}"
  )
endif()
