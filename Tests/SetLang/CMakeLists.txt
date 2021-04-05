# test forcing a source file language to c++ from c
cmake_minimum_required (VERSION 2.6)
project(SetLang)
# force this to be verbose so I can debug a dashboard entry
set(CMAKE_VERBOSE_MAKEFILE 1)
add_library(foo foo.c)
add_executable(SetLang bar.c)
set_source_files_properties(foo.c bar.c PROPERTIES LANGUAGE CXX)
target_link_libraries(SetLang foo)
set_target_properties(SetLang PROPERTIES LINKER_LANGUAGE CXX)

# VS generators historically tolerated target-wide -TP flags added
# by project code, so cover that case to preserve the behavior.
if(CMAKE_GENERATOR MATCHES "^Visual Studio" AND "x${CMAKE_C_COMPILER_ID}" STREQUAL "xMSVC")
  add_library(stay stay_c.c stay_cxx.cxx)
  set_property(TARGET stay PROPERTY COMPILE_OPTIONS "-TP")
endif()
