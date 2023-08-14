enable_language(C)

set(info "")
foreach(var
    CMAKE_C_IMPLICIT_LINK_DIRECTORIES
    )
  if(DEFINED ${var})
    string(APPEND info "set(INFO_${var} \"${${var}}\")\n")
  endif()
endforeach()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake" "${info}")
