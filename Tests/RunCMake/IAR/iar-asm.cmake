enable_language(ASM)

add_executable(exec-asm module.asm)
target_link_options(exec-asm PRIVATE ${LINKER_OPTS})
