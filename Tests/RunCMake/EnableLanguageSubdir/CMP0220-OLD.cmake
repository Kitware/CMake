
# With CMP0220 OLD, a language enabled in a subdirectory is not propagated
cmake_policy(SET CMP0220 OLD)

add_subdirectory(cxx)

if(CMAKE_CXX_COMPILER_LOADED)
  message(FATAL_ERROR
    "enable_language(): language configuration was propagated up a subdirectory with CMP0220 OLD")
endif()
