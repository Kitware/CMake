GET_CURRENT_FILE(current_file)
IF(NOT "${current_file}" STREQUAL "${CMAKE_CURRENT_LIST_FILE}")
  MESSAGE(FATAL_ERROR
    "Macro file context is broken.  Expected:\n"
    "  ${CMAKE_CURRENT_LIST_FILE}\n"
    "but got:\n"
    "  ${current_file}\n"
    "from the macro."
    )
ENDIF(NOT "${current_file}" STREQUAL "${CMAKE_CURRENT_LIST_FILE}")
