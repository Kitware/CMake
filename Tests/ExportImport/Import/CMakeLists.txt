cmake_minimum_required (VERSION 2.7.20090711)
project(Import C CXX)

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

# Test package INTERFACE controls
add_subdirectory(package_old_old)
add_subdirectory(package_new_old)
add_subdirectory(package_new_new)
