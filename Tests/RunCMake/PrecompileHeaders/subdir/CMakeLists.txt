add_executable(foobar ../foobar.c)
target_link_libraries(foobar foo )
set_target_properties(foobar PROPERTIES PRECOMPILE_HEADERS_REUSE_FROM foo)
