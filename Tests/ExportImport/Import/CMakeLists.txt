cmake_minimum_required (VERSION 2.7.20090711)
cmake_policy(SET CMP0025 NEW)
if(POLICY CMP0129)
  cmake_policy(SET CMP0129 NEW)
endif()
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

add_subdirectory(try_compile)

# Test package INTERFACE controls
add_subdirectory(Interface)

# Test package version range
add_subdirectory(version_range)

# Test install(IMPORTED_RUNTIME_ARTIFACTS)
add_subdirectory(install-IMPORTED_RUNTIME_ARTIFACTS)

# Test install(RUNTIME_DEPENDENCIES) and install(RUNTIME_DEPENDENCY_SET)
if(CMAKE_SYSTEM_NAME MATCHES "^(Linux|Windows|Darwin)$")
  add_subdirectory(install-RUNTIME_DEPENDENCIES)
  add_subdirectory(install-RUNTIME_DEPENDENCY_SET)
endif()
