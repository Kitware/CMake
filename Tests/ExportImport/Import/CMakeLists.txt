cmake_minimum_required (VERSION 2.6)
project(Import C)

# We need ansi C support.
if(CMAKE_ANSI_CFLAGS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")
endif(CMAKE_ANSI_CFLAGS)

# Import everything in a subdirectory.
add_subdirectory(A)

# Make sure the imported targets are scoped inside the subdirectory.
if(TARGET exp_testLib2)
  message(FATAL_ERROR "Imported target exp_testLib2 is not scoped in subdir!")
endif()
if(TARGET bld_testLib2)
  message(FATAL_ERROR "Imported target bld_testLib2 is not scoped in subdir!")
endif()

# Test transitive linking to a target imported in the subdirectory.
add_executable(imp_testTransExe1 imp_testTransExe1.c)
target_link_libraries(imp_testTransExe1 imp_lib1)
add_executable(imp_testTransExe1b imp_testTransExe1.c)
target_link_libraries(imp_testTransExe1b imp_lib1b)
