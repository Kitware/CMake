enable_language(C)
enable_language(CXX)
if(CMake_TEST_Fortran)
  enable_language(Fortran)
endif()

set(info "")
foreach(var
    CMAKE_SYSTEM_NAME
    CMAKE_C_COMPILER_ID
    CMAKE_C_COMPILER_VERSION
    CMAKE_CXX_COMPILER_ID
    CMAKE_CXX_COMPILER_VERSION
    CMAKE_Fortran_COMPILER_ID
    CMAKE_Fortran_COMPILER_VERSION
    )
  if(DEFINED ${var})
    string(APPEND info "set(${var} \"${${var}}\")\n")
  endif()
endforeach()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake" "${info}")
