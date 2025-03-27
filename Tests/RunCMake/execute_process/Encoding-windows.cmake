if(CMP0176 STREQUAL "NEW")
  cmake_policy(SET CMP0176 NEW)
endif()

# Set the console code page.
execute_process(COMMAND cmd /c chcp ${CODEPAGE})

include(${CMAKE_CURRENT_LIST_DIR}/Encoding-common.cmake)

# Save our internal UTF-8 representation of the output.
file(WRITE "out.txt" "${out}")
