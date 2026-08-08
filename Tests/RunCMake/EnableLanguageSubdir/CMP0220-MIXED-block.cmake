
# enable_language() policy check should respect the policy inside of block()
cmake_policy(SET CMP0220 NEW)

block()
  cmake_policy(SET CMP0220 OLD)
  add_subdirectory(cxx)

  if(CMAKE_CXX_COMPILER_LOADED)
    message(FATAL_ERROR
      "enable_language(): language configuration was incorrectly propagated up a subdirectory with CMP0220 OLD inside a block()")
  endif()
endblock()
