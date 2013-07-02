
cmake_minimum_required(VERSION 2.8)

project(SystemIncludeDirectories)

add_library(systemlib systemlib.cpp)
target_include_directories(systemlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/systemlib")

add_library(upstream upstream.cpp)
target_link_libraries(upstream LINK_PUBLIC systemlib)
target_compile_options(upstream PRIVATE -Werror=unused-variable)

target_include_directories(upstream SYSTEM PUBLIC
  $<TARGET_PROPERTY:systemlib,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(consumer consumer.cpp)
target_link_libraries(consumer upstream)
target_compile_options(consumer PRIVATE -Werror=unused-variable)
