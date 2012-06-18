
cmake_minimum_required(VERSION 2.8)

project(TargetIncludeDirectories)

macro(create_header _name)
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_name}")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${_name}/${_name}.h" "//${_name}.h\n")
endmacro()

create_header(bar)
create_header(bat)
create_header(foo)
create_header(baz)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/bar")

add_executable(TargetIncludeDirectories main.cpp)
set_property(TARGET TargetIncludeDirectories APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/bat")
set_property(TARGET TargetIncludeDirectories APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/foo")

include_directories("${CMAKE_CURRENT_BINARY_DIR}/baz")
