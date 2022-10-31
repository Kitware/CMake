enable_language(CXX)
if(CMake_TEST_OBJC)
  enable_language(OBJC)
  enable_language(OBJCXX)
endif()

set(info "")
foreach(var
    CMAKE_CXX_EXTENSIONS_DEFAULT
    CMAKE_OBJC_STANDARD_DEFAULT
    CMAKE_OBJCXX_STANDARD_DEFAULT
    )
  if(DEFINED ${var})
    string(APPEND info "set(${var} \"${${var}}\")\n")
  endif()
endforeach()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake" "${info}")
