include(common.cmake)

set(file "")

file_download()

set(file "${CMAKE_CURRENT_BINARY_DIR}/input.png")

if(NOT EXISTS "${file}")
  message(FATAL_ERROR "file not downloaded to expected path:\n ${file}")
endif()
