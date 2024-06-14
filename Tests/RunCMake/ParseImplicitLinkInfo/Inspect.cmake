enable_language(C)

set(info "")
foreach(var
    CMAKE_SYSTEM_NAME
    CMAKE_C_COMPILER
    CMAKE_C_COMPILER_ID
    CMAKE_C_COMPILER_VERSION
    CMAKE_C_COMPILER_LINKER
    CMAKE_C_COMPILER_LINKER_ID
    CMAKE_C_COMPILER_LINKER_VERSION
    CMAKE_C_IMPLICIT_LINK_DIRECTORIES
    )
  if(DEFINED ${var})
    string(APPEND info "set(${var} \"${${var}}\")\n")
  endif()
endforeach()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake" "${info}")
