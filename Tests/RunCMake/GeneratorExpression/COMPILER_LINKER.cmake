
set(languages C ${LANG})
list(REMOVE_DUPLICATES languages)

enable_language(${languages})

include(CTest)

set(VAR "${CMAKE_${LANG}_COMPILER_LINKER_${TYPE}}")
if(NOT VAR)
  set(VAR "UNDEF")
endif()

add_executable(COMPILER_LINKER compiler_linker.c)
target_compile_definitions(COMPILER_LINKER PRIVATE "VAR=${VAR}"
                                                   "GENEX=$<IF:$<BOOL:$<${LANG}_COMPILER_LINKER_${TYPE}>>,$<${LANG}_COMPILER_LINKER_${TYPE}>,UNDEF>")

add_test(NAME COMPILER_LINKER.${LANG}.${TYPE}
         COMMAND COMPILER_LINKER)
