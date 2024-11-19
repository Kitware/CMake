enable_language(C)

set(info "")
foreach(var
    CMAKE_C_LINK_LIBRARIES_PROCESSING
    CMAKE_C_PLATFORM_LINKER_ID
    CMAKE_EXECUTABLE_FORMAT
    )
  if(DEFINED ${var})
    string(APPEND info "set(${var} \"${${var}}\")\n")
  endif()
endforeach()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake" "${info}")
