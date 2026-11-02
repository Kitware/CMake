# test forcing a source file language to c++ from c
cmake_minimum_required(VERSION 3.10)
if(POLICY CMP0129)
  cmake_policy(SET CMP0129 NEW)
endif()
project(SetLang)
# force this to be verbose so I can debug a dashboard entry
set(CMAKE_VERBOSE_MAKEFILE 1)
if(CMAKE_C_COMPILER_ID MATCHES "(GNU|LCC|Clang|MSVC|Borland|Embarcadero|Intel|TI|XL)")
  set(have_explicit_language 1)
  cmake_policy(SET CMP0119 NEW)
endif()
add_library(foo foo.c)
add_executable(SetLang bar.c)
set_source_files_properties(foo.c bar.c PROPERTIES LANGUAGE CXX)
target_link_libraries(SetLang foo)
set_target_properties(SetLang PROPERTIES LINKER_LANGUAGE CXX)

# Windows defender blocks SetLang executable (https://cygwin.com/pipermail/cygwin/2021-January/247508.html)
# Coordinate this name with 'Tests/CMakeLists.txt'.
set_target_properties(SetLang PROPERTIES OUTPUT_NAME "SetLangX")

# VS generators historically tolerated target-wide -TP flags added
# by project code, so cover that case to preserve the behavior.
if(CMAKE_GENERATOR MATCHES "^Visual Studio" AND "x${CMAKE_C_COMPILER_ID}" STREQUAL "xMSVC")
  add_library(stay stay_c.c stay_cxx.cxx)
  set_property(TARGET stay PROPERTY COMPILE_OPTIONS "-TP")
endif()

if(have_explicit_language)
  add_library(zoom zoom.zzz)
  set_source_files_properties(zoom.zzz PROPERTIES LANGUAGE CXX)
  target_link_libraries(SetLang zoom)
  target_compile_definitions(SetLang PRIVATE WITH_ZOOM)
endif()
