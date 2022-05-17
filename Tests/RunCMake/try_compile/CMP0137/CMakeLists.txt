cmake_minimum_required(VERSION 3.23)
project(TestCMP0137 NONE)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake" "
message(STATUS \"CMAKE_ENABLE_EXPORTS='${CMAKE_ENABLE_EXPORTS}'\")
message(STATUS \"FOO='${FOO}'\")
")
