enable_language(C)

include(CheckCompilerFlag)

# Confirm we can check the conflicting flag directly. This should pass with
# or without the workaround.
check_compiler_flag(C "-fembed-bitcode" result1)
if(NOT result1)
  message(FATAL_ERROR "False negative when -fembed-bitcode tested directly")
endif()

# Check conflicting flag set by user or project won't cause a false negative
# when testing a valid flag. This only passes with the workaround.
set(CMAKE_C_FLAGS -fembed-bitcode)
check_compiler_flag(C "-O" result2)
if(NOT result2)
  message(FATAL_ERROR "False negative when -fembed-bitcode set in CMAKE_C_FLAGS")
endif()
