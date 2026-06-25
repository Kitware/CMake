
# enable_language() propagates through a function call
cmake_policy(SET CMP0220 NEW)

function(enable_cxx)
  enable_language(CXX)
endfunction()

enable_cxx()

if(NOT CMAKE_CXX_COMPILER_LOADED)
  message(FATAL_ERROR
    "enable_language(): language configuration was not propagated up a subdirectory with CMP0220 NEW inside a function()")
endif()
