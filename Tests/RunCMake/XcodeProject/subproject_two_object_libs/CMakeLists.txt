cmake_minimum_required (VERSION 3.16)

enable_language(CXX)

project (subproject_two_object_libs)

set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED NO)
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")

if(IOS)
  set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

# Hierarchy of libraries looks like:
#
# OBJECT-1 ---> STATIC--\
#                        |---> SHARED
# OBJECT-2 -------------/

add_library(object_lib_dependency1 OBJECT dummy.cpp)

add_library(static_lib_dependency STATIC dummy.cpp $<TARGET_OBJECTS:object_lib_dependency1>)

add_library(object_lib_dependency2 OBJECT dummy.cpp)
target_link_libraries(object_lib_dependency2 PRIVATE static_lib_dependency)

add_library(shared_lib SHARED dummy.cpp)
target_link_libraries(shared_lib PRIVATE static_lib_dependency)
target_sources(shared_lib PRIVATE $<TARGET_OBJECTS:object_lib_dependency2>)
