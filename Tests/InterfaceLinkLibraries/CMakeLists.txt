cmake_minimum_required (VERSION 2.8)

cmake_policy(SET CMP0022 NEW)

project(InterfaceLinkLibraries)

add_library(foo_shared SHARED foo.cpp)
target_compile_definitions(foo_shared INTERFACE FOO_LIBRARY)
add_library(bar_shared SHARED bar.cpp)
target_compile_definitions(bar_shared INTERFACE BAR_LIBRARY)
set_property(TARGET bar_shared APPEND PROPERTY INTERFACE_LINK_LIBRARIES foo_shared)

add_executable(shared_test main.cpp)
set_property(TARGET shared_test APPEND PROPERTY LINK_LIBRARIES bar_shared)

add_library(foo_static STATIC foo.cpp)
target_compile_definitions(foo_static INTERFACE FOO_LIBRARY)
add_library(bar_static STATIC bar.cpp)
target_compile_definitions(bar_static INTERFACE BAR_LIBRARY)
set_property(TARGET bar_static APPEND PROPERTY INTERFACE_LINK_LIBRARIES foo_static)

add_executable(static_test main.cpp)
set_property(TARGET static_test APPEND PROPERTY LINK_LIBRARIES bar_static)

add_library(foo_shared_private SHARED foo.cpp)
target_compile_definitions(foo_shared_private INTERFACE FOO_LIBRARY)
add_library(bang_shared_private SHARED bang.cpp)
target_compile_definitions(bang_shared_private INTERFACE BANG_LIBRARY)
add_library(bar_shared_private SHARED bar.cpp)
target_compile_definitions(bar_shared_private INTERFACE BAR_LIBRARY)
target_compile_definitions(bar_shared_private PRIVATE BAR_USE_BANG)
set_property(TARGET bar_shared_private APPEND PROPERTY LINK_LIBRARIES bang_shared_private)
set_property(TARGET bar_shared_private APPEND PROPERTY INTERFACE_LINK_LIBRARIES foo_shared_private)

add_executable(shared_private_test main.cpp)
set_property(TARGET shared_private_test APPEND PROPERTY LINK_LIBRARIES bar_shared_private)

add_library(foo_static_private STATIC foo.cpp)
target_compile_definitions(foo_static_private INTERFACE FOO_LIBRARY)
add_library(bang_static_private STATIC bang.cpp)
target_compile_definitions(bang_static_private INTERFACE BANG_LIBRARY)
add_library(bar_static_private STATIC bar.cpp)
target_compile_definitions(bar_static_private INTERFACE BAR_LIBRARY)
target_compile_definitions(bar_static_private PRIVATE BAR_USE_BANG)
set_property(TARGET bar_static_private APPEND PROPERTY LINK_LIBRARIES bang_static_private)
set_property(TARGET bar_static_private APPEND PROPERTY INTERFACE_LINK_LIBRARIES $<LINK_ONLY:bang_static_private> foo_static_private)

add_executable(InterfaceLinkLibraries main.cpp)
set_property(TARGET InterfaceLinkLibraries APPEND PROPERTY LINK_LIBRARIES bar_static_private)
