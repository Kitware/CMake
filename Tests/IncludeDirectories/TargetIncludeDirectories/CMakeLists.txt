
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
create_header(bang)
create_header(bing)
create_header(bung)
create_header(arguments)
create_header(list)
create_header(target)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/bar")
include_directories("$<1:${CMAKE_CURRENT_BINARY_DIR}/bang>")

add_executable(TargetIncludeDirectories main.cpp)

set_property(TARGET TargetIncludeDirectories APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/bat")
set_property(TARGET TargetIncludeDirectories APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/foo")
set_property(TARGET TargetIncludeDirectories APPEND PROPERTY
    INCLUDE_DIRECTORIES "$<1:${CMAKE_CURRENT_BINARY_DIR}/bing>")

include_directories("${CMAKE_CURRENT_BINARY_DIR}/baz")
include_directories("$<1:${CMAKE_CURRENT_BINARY_DIR}/bung>")
include_directories("sing$<1:/ting>")

include_directories("$<1:${CMAKE_CURRENT_BINARY_DIR}/arguments;${CMAKE_CURRENT_BINARY_DIR}/list>")

add_library(somelib::withcolons UNKNOWN IMPORTED)
set_property(TARGET somelib::withcolons PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/target")
set_property(TARGET somelib::withcolons PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/target")

set_property(TARGET TargetIncludeDirectories
  APPEND PROPERTY INCLUDE_DIRECTORIES
  "$<TARGET_PROPERTY:somelib::withcolons,INTERFACE_INCLUDE_DIRECTORIES>"
)
