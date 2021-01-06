if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/exists-for-build2")
  message(FATAL_ERROR "Custom command incorrectly re-ran after CMake re-ran!")
endif()
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/out.txt")
