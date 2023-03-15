include(common.cmake)

# Test downloading without saving to a file.
set(file "")
file_download()

foreach(name input.png output.png TIMEOUT)
  if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${name}")
    message(FATAL_ERROR "file incorrectly saved to:\n ${CMAKE_CURRENT_BINARY_DIR}/${name}")
  endif()
endforeach()
