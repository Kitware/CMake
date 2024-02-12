enable_language(ASM)

add_executable(exec-asm)
target_sources(exec-asm PRIVATE module.s)
target_link_options(exec-asm PRIVATE ${LINKER_OPTS})
