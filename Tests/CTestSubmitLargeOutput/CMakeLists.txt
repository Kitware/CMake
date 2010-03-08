cmake_minimum_required(VERSION 2.8)
project(SubmitLargeOutput NONE)

include(CTest)

add_test(GenerateRandomOutput ${CMAKE_COMMAND}
  -P ${CMAKE_CURRENT_SOURCE_DIR}/GenerateRandomOutput.cmake
  )
