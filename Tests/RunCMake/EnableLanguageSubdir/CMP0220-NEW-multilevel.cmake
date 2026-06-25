
# Ensure propagation through many subdirectories each with multiple scopes
cmake_policy(SET CMP0220 NEW)

add_subdirectory(outer)

if(NOT CMAKE_CXX_COMPILER_LOADED)
  message(FATAL_ERROR
    "enable_language(): language configuration was not propagated up across "
    "multiple levels of directories.")
endif()
