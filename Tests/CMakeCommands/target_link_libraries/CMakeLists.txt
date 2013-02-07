cmake_minimum_required(VERSION 2.8)

project(target_link_libraries)

file(WRITE
  "${CMAKE_CURRENT_BINARY_DIR}/main.cxx"
  "int main() { return 0; }
"
)

add_executable(
  target_link_libraries
  "${CMAKE_CURRENT_BINARY_DIR}/main.cxx"
)

macro(ASSERT_PROPERTY _target _property _value)
  get_target_property(_out ${_target} ${_property})
  if (NOT _out)
    set(_out "")
  endif()
  if (NOT "${_out}" STREQUAL "${_value}")
    message(SEND_ERROR "Target ${_target} does not have property ${_property} with value ${_value}. Actual value: ${_out}")
  endif()
endmacro()

include(GenerateExportHeader)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(depA SHARED depA.cpp)
generate_export_header(depA)

add_library(depB SHARED depB.cpp)
generate_export_header(depB)

target_link_libraries(depB LINK_PRIVATE depA)

add_library(libgenex SHARED libgenex.cpp)
generate_export_header(libgenex)

set_property(TARGET depB APPEND PROPERTY
  LINK_LIBRARIES $<1:libgenex>
)

add_library(depC SHARED depC.cpp)
generate_export_header(depC)

target_link_libraries(depC LINK_PUBLIC depA)

assert_property(depA LINK_INTERFACE_LIBRARIES "")
assert_property(depB LINK_INTERFACE_LIBRARIES "")
assert_property(depC LINK_INTERFACE_LIBRARIES "depA")

add_executable(targetA targetA.cpp)

target_link_libraries(targetA LINK_INTERFACE_LIBRARIES depA depB)

assert_property(targetA LINK_INTERFACE_LIBRARIES "depA;depB")

set_target_properties(targetA PROPERTIES LINK_INTERFACE_LIBRARIES "")

assert_property(targetA LINK_INTERFACE_LIBRARIES "")

add_subdirectory(subdir)
target_link_libraries(targetA subdirlib)

target_link_libraries(targetA depB depC)

assert_property(targetA LINK_INTERFACE_LIBRARIES "")

# Exclude depIfaceOnly from ALL so that it will only be built if something
# depends on it. As it is in the link interface of depB, targetA
# will depend on it. That dependency is what is being tested here.
add_library(depIfaceOnly SHARED EXCLUDE_FROM_ALL depIfaceOnly.cpp)
generate_export_header(depIfaceOnly)
set_property(TARGET depB APPEND PROPERTY LINK_INTERFACE_LIBRARIES depIfaceOnly)

add_library(depD SHARED depD.cpp)
generate_export_header(depD)
set_property(TARGET depD APPEND PROPERTY
  LINK_INTERFACE_LIBRARIES
    $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:depA>
)

add_executable(targetB targetB.cpp)
target_link_libraries(targetB depD)

macro(create_header _name)
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_name}")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${_name}/${_name}.h" "//${_name}.h\n")
endmacro()

create_header(foo)
create_header(bar)

add_library(depG SHARED depG.cpp)
generate_export_header(depG)
target_include_directories(depG INTERFACE
    "${CMAKE_CURRENT_BINARY_DIR}/foo"
    "${CMAKE_CURRENT_BINARY_DIR}/bar"
)
target_compile_definitions(depG INTERFACE
    TEST_DEF
)

add_executable(targetC targetC.cpp)
# Creates a generator expression for include directories like
#  $<$<TARGET_DEFINED:$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:depG>>:\
#    $<TARGET_PROPERTY:$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:depG>,INTERFACE_INCLUDE_DIRECTORIES>>
target_link_libraries(targetC $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:depG>)
