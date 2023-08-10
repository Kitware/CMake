enable_language(CXX)

set(info "")
foreach(var
    CMAKE_INCLUDE_FLAG_CXX
    CMAKE_INCLUDE_SYSTEM_FLAG_CXX
    )
  if(DEFINED ${var})
    string(APPEND info "set(${var} \"${${var}}\")\n")
  endif()
endforeach()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake" "${info}")
