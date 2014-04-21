cmake_minimum_required(VERSION 3.0)
project(target_compile_features)

if (NOT CMAKE_CXX_COMPILE_FEATURES)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/test_dummy.cpp"
    "int main(int,char**) { return 0; }\n"
  )
  add_executable(target_compile_features "${CMAKE_CURRENT_BINARY_DIR}/test_dummy.cpp")
  return()
endif()

set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(target_compile_features main.cpp)
target_compile_features(target_compile_features
  PRIVATE cxx_auto_type
)

add_library(lib_auto_type lib_auto_type.cpp)
target_compile_features(lib_auto_type
  PUBLIC cxx_auto_type
)

add_executable(lib_user lib_user.cpp)
target_link_libraries(lib_user lib_auto_type)
