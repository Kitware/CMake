cmake_minimum_required(VERSION 2.8)
project(ObjectLibrary C)

add_subdirectory(A)
add_subdirectory(B)

add_library(Cstatic STATIC c.c $<TARGET_OBJECTS:A> $<TARGET_OBJECTS:B>)
add_executable(UseCstatic main.c)
target_link_libraries(UseCstatic Cstatic)

add_library(Cshared SHARED c.c $<TARGET_OBJECTS:A> $<TARGET_OBJECTS:Bexport>)
add_executable(UseCshared main.c)
set_property(TARGET UseCshared PROPERTY COMPILE_DEFINITIONS SHARED_C)
target_link_libraries(UseCshared Cshared)

add_executable(UseCinternal main.c c.c $<TARGET_OBJECTS:A> $<TARGET_OBJECTS:B>)
