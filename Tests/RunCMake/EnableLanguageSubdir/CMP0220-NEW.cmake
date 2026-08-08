
# With CMP0220 NEW, a language enabled in a subdirectory is propagated up to this scope
cmake_policy(SET CMP0220 NEW)

add_subdirectory(cxx)

if(NOT CMAKE_CXX_COMPILER_LOADED)
  message(FATAL_ERROR
      "enable_language(): language configuration was not propagated up a subdirectory with CMP0220 NEW")
endif()
