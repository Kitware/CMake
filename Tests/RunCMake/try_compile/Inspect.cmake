enable_language(C)
enable_language(CXX)
if(CMake_TEST_OBJC)
  enable_language(OBJC)
  enable_language(OBJCXX)
endif()

set(info "")
foreach(var
    CMAKE_C_COMPILER_ID
    CMAKE_C_COMPILER_VERSION
    CMAKE_C_STANDARD_DEFAULT
    CMAKE_CXX_COMPILER_ID
    CMAKE_CXX_COMPILER_VERSION
    CMAKE_CXX_STANDARD_DEFAULT
    CMAKE_CXX_EXTENSIONS_DEFAULT
    CMAKE_OBJC_STANDARD_DEFAULT
    CMAKE_OBJCXX_STANDARD_DEFAULT
    )
  if(DEFINED ${var})
    string(APPEND info "set(${var} \"${${var}}\")\n")
  endif()
endforeach()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake" "${info}")
