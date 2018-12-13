project(Alias)
enable_language(CXX)

add_library(c_alias_lib ALIAS c_lib)
add_executable(c_alias_exe ../empty.c)
target_link_libraries(c_alias_exe PRIVATE c_alias_lib)

add_library(cxx_alias_lib ALIAS cxx_lib)
add_executable(cxx_alias_exe ../empty.cxx)
target_link_libraries(cxx_alias_exe PRIVATE cxx_alias_lib)
