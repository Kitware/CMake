cmake_minimum_required (VERSION 2.6)
project(Assembler)

set(SRCS)

# (at least) the following toolchains xan process assembler files directly
# and also generate assembler files from C:
if(CMAKE_C_COMPILER_ID STREQUAL "GNU"
   OR CMAKE_C_COMPILER_ID STREQUAL "HP"
   OR CMAKE_C_COMPILER_ID STREQUAL "Intel"
   OR CMAKE_C_COMPILER_ID STREQUAL "SunPro"
   OR CMAKE_C_COMPILER_ID STREQUAL "XL")
  execute_process(COMMAND ${CMAKE_C_COMPILER} -S "${CMAKE_CURRENT_SOURCE_DIR}/main.c" -o "${CMAKE_CURRENT_BINARY_DIR}/main.s")
  set(SRCS "${CMAKE_CURRENT_BINARY_DIR}/main.s")
endif(CMAKE_C_COMPILER_ID STREQUAL "GNU"
   OR CMAKE_C_COMPILER_ID STREQUAL "HP"
   OR CMAKE_C_COMPILER_ID STREQUAL "Intel"
   OR CMAKE_C_COMPILER_ID STREQUAL "SunPro"
   OR CMAKE_C_COMPILER_ID STREQUAL "XL")


if(SRCS)
  enable_language(ASM OPTIONAL)
else(SRCS)
  message(STATUS "No assembler enabled, using C")
  set(SRCS main.c)
endif(SRCS)

add_executable(HelloAsm ${SRCS})
