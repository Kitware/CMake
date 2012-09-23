
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

create_header(fee)
create_header(fiy)
create_header(foh)
create_header(fum)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/lib1.cpp" "#include \"fee.h\"")
add_library(lib1 "${CMAKE_CURRENT_BINARY_DIR}/lib1.cpp")
set_property(TARGET lib1 APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/fee")
set_property(TARGET lib1 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/fiy")
set_property(TARGET lib1 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${CMAKE_CURRENT_BINARY_DIR}/foh>")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/lib2.cpp" "#include \"fiy.h\"")
add_library(lib2 "${CMAKE_CURRENT_BINARY_DIR}/lib2.cpp")
set_property(TARGET lib2 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/fum;$<TARGET_PROPERTY:lib1,INTERFACE_INCLUDE_DIRECTORIES>")
set_property(TARGET lib2 APPEND PROPERTY INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:lib1,INTERFACE_INCLUDE_DIRECTORIES>")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/main3.cpp" "#include \"fiy.h\"\n#include \"foh.h\"\n#include \"fum.h\"\nint main(int,char**) { return 0; }\n")
add_executable(exe3 "${CMAKE_CURRENT_BINARY_DIR}/main3.cpp")
set_property(TARGET exe3 APPEND PROPERTY INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:lib2,INTERFACE_INCLUDE_DIRECTORIES>")
