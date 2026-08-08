
# Policy is checked at every scope level for CXX and its implicit RC language.
cmake_policy(SET CMP0220 NEW)

block()
  cmake_policy(SET CMP0220 OLD)
  block()
    cmake_policy(SET CMP0220 NEW)
    add_subdirectory(cxx)

    if(NOT CMAKE_CXX_COMPILER_LOADED)
      message(FATAL_ERROR
        "enable_language(): language configuration was not propagated up a subdirectory with CMP0220 OLD inside a block()")
    endif()
    if(NOT CMAKE_RC_COMPILER_LOADED)
      message(FATAL_ERROR
        "enable_language(): implicit RC language configuration was not propagated up a subdirectory with CMP0220 OLD inside a block()")
    endif()
  endblock()

  if(CMAKE_CXX_COMPILER_LOADED)
    message(FATAL_ERROR
      "enable_language(): language configuration was incorrectly propagated up a subdirectory with CMP0220 OLD inside a block()")
  endif()
  if(CMAKE_RC_COMPILER_LOADED)
    message(FATAL_ERROR
      "enable_language(): implicit RC language configuration was incorrectly propagated up a subdirectory with CMP0220 OLD inside a block()")
  endif()
endblock()
