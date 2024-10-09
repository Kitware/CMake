enable_language(C)
enable_language(CXX)

set(info "")
foreach(var
    CMAKE_C_STANDARD_DEFAULT
    CMAKE_CXX_STANDARD_DEFAULT
    )
  if(DEFINED ${var})
    string(APPEND info "set(${var} \"${${var}}\")\n")
  endif()
endforeach()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake" "${info}")
