cmake_minimum_required(VERSION 2.8)
project(MacRuntimePath_B)

include(${MacRuntimePath_B_BINARY_DIR}/../Root/lib/exp.cmake)

add_executable(testb ${MacRuntimePath_B_SOURCE_DIR}/../A/test3.cpp)

# test link with rpath enabled targets
target_link_libraries(testb shared framework)

# test link with rpath enabled library by filename
target_link_libraries(testb $<TARGET_LINKER_FILE:shared2> framework)

add_custom_target(testb_run ALL
  COMMAND testb
  DEPENDS testb
  )
