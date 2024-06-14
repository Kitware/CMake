cmake_minimum_required(VERSION 3.29)

enable_language(C)

set(CMAKE_LINKER_TYPE DEFAULT)
set(CMAKE_C_USING_LINKER_abc123 /path/to/somewhere)
set(CMAKE_C_USING_LINKER_Hi_There some-tool)
set(CMAKE_ASM_NASM_USING_LINKER_custom /place/holder)
set(CMAKE_ASM-ATT_USING_LINKER_custom /more/text)
set(CMAKE_ASM-ATT_USING_LINKER_MODE TOOL)

try_compile(RESULT
  PROJECT TestProject
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proj_vars
  OUTPUT_VARIABLE output
)

message(STATUS "\n${output}")
