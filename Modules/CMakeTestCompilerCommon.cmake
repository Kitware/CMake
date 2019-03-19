# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


function(PrintTestCompilerStatus LANG MSG)
  message(STATUS "Check for working ${LANG} compiler: ${CMAKE_${LANG}_COMPILER}${MSG}")
endfunction()

# if required set the target type if not already explicitly set
macro(__TestCompiler_setTryCompileTargetType)
  if(NOT CMAKE_TRY_COMPILE_TARGET_TYPE)
    if("${CMAKE_GENERATOR}" MATCHES "Green Hills MULTI")
      #prefer static libraries to avoid linking issues
      set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
      set(__CMAKE_TEST_COMPILER_TARGET_TYPE_RESTORE 1)
    endif()
  endif()
endmacro()

# restore the original value
# -- not necessary if __TestCompiler_setTryCompileTargetType() was used in function scope
macro(__TestCompiler_restoreTryCompileTargetType)
  if(__CMAKE_TEST_COMPILER_TARGET_TYPE_RESTORE)
    unset(CMAKE_TRY_COMPILE_TARGET_TYPE)
    unset(__CMAKE_TEST_COMPILER_TARGET_TYPE_RESTORE)
  endif()
endmacro()
