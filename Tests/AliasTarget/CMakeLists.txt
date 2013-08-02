
cmake_minimum_required(VERSION 2.8.11)
project(AliasTarget)

add_library(foo SHARED empty.cpp)
add_library(PREFIX::Foo ALIAS foo)
add_library(Another::Alias ALIAS foo)

target_compile_definitions(foo PUBLIC FOO_DEFINE)

add_library(bar SHARED empty.cpp)
target_compile_definitions(bar PUBLIC BAR_DEFINE)

target_link_libraries(foo LINK_PUBLIC $<$<STREQUAL:$<TARGET_PROPERTY:PREFIX::Foo,ALIASED_TARGET>,foo>:bar>)

add_executable(AliasTarget empty.cpp)
add_executable(PREFIX::AliasTarget ALIAS AliasTarget)

add_library(bat SHARED bat.cpp)
target_link_libraries(bat PREFIX::Foo)

if (NOT TARGET Another::Alias)
  message(SEND_ERROR "Another::Alias is not considered a target.")
endif()

get_target_property(_alt PREFIX::Foo ALIASED_TARGET)
if (NOT ${_alt} STREQUAL foo)
  message(SEND_ERROR "ALIASED_TARGET is not foo: ${_alt}")
endif()

get_property(_alt2 TARGET PREFIX::Foo PROPERTY ALIASED_TARGET)
if (NOT ${_alt2} STREQUAL foo)
  message(SEND_ERROR "ALIASED_TARGET is not foo.")
endif()
