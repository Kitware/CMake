
cmake_minimum_required(VERSION 2.8.11)
project(Foo)

add_library(foo SHARED empty.cpp)
add_library(PREFIX::Foo ALIAS foo)
add_library(Another::Alias ALIAS foo)

target_compile_definitions(foo PUBLIC FOO_DEFINE)

add_library(bar SHARED empty.cpp)
target_compile_definitions(bar PUBLIC BAR_DEFINE)

target_link_libraries(foo LINK_PUBLIC bar)

add_executable(AliasTarget empty.cpp)
add_executable(PREFIX::AliasTarget ALIAS AliasTarget)

add_library(bat SHARED bat.cpp)
target_link_libraries(bat PREFIX::Foo)
