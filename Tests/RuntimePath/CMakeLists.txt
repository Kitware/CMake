cmake_minimum_required (VERSION 3.15)
project(RuntimePath C)

# Add a simple chain of shared libraries that must be found.
add_library(foo1 SHARED foo1.c)
set_property(TARGET foo1 PROPERTY OUTPUT_NAME foo)
set_property(TARGET foo1 PROPERTY LIBRARY_OUTPUT_DIRECTORY A)

add_library(bar1 SHARED bar1.c)
set_property(TARGET bar1 PROPERTY OUTPUT_NAME bar)
set_property(TARGET bar1 PROPERTY VERSION 1)
set_property(TARGET bar1 PROPERTY LIBRARY_OUTPUT_DIRECTORY B)
target_link_libraries(bar1 foo1)

add_executable(RuntimePath main.c)
target_link_libraries(RuntimePath bar1)

# Add a library that provides a conflicting location to make sure
# rpath ordering works.
add_library(foo2 SHARED foo2.c)
set_property(TARGET foo2 PROPERTY OUTPUT_NAME foo)
set_property(TARGET foo2 PROPERTY LIBRARY_OUTPUT_DIRECTORY B)

# Add a library that would provide a conflicting location if not for
# soname analysis in rpath ordering.  This will also break the old
# link directory ordering to make sure files are linked with full
# paths.
if(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG)
  add_library(bar2 SHARED bar2.c)
  set_property(TARGET bar2 PROPERTY OUTPUT_NAME bar)
  set_property(TARGET bar2 PROPERTY LIBRARY_OUTPUT_DIRECTORY A)
  target_link_libraries(bar2 foo2)
endif()

# Add a library that is missing the rpath for its dependency.
add_library(bar1_no_rpath SHARED bar1.c)
set_property(TARGET bar1_no_rpath PROPERTY LIBRARY_OUTPUT_DIRECTORY B)
set_property(TARGET bar1_no_rpath PROPERTY SKIP_BUILD_RPATH 1)
target_link_libraries(bar1_no_rpath PRIVATE foo1)

# Add an executable linking to the library with a missing dependency rpath.
# CMake should generate the proper rpath-link flag to find it at build time.
add_executable(main_with_bar1_no_rpath main.c)
target_link_libraries(main_with_bar1_no_rpath bar1_no_rpath)
