# test forcing a source file language to c++ from c
project(SetLang)
add_library(foo foo.c)
add_executable(bar bar.c)
set_source_files_properties(foo.c bar.c PROPERTIES LANGUAGE CXX)
target_link_libraries(bar foo)
set_target_properties(bar PROPERTIES LINKER_LANGUAGE CXX)
