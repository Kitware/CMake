enable_language(C)

add_executable(exec-c)
target_sources(exec-c PRIVATE module.c)
target_link_options(exec-c PRIVATE ${LINKER_OPTS})
