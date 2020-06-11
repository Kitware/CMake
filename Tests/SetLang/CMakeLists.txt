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

if((CMAKE_C_COMPILER_ID MATCHES "(GNU|Clang|MSVC|Borland|Embarcadero|Intel|TI|XL)"))
  add_library(zoom zoom.zzz)
  set_source_files_properties(zoom.zzz PROPERTIES LANGUAGE CXX)
  target_link_libraries(SetLang zoom)
  target_compile_definitions(SetLang PRIVATE WITH_ZOOM)
endif()
