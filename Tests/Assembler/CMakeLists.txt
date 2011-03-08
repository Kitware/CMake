cmake_minimum_required (VERSION 2.6)
project(Assembler C)
message("CTEST_FULL_OUTPUT ")
set(CMAKE_VERBOSE_MAKEFILE 1)

set(SRCS)

# (at least) the following toolchains can process assembler files directly
# and also generate assembler files from C:
if("${CMAKE_GENERATOR}" MATCHES "Makefile")
  if(("${CMAKE_C_COMPILER_ID}" MATCHES "^(GNU|HP|SunPro|XL)$") OR ("${CMAKE_C_COMPILER_ID}" STREQUAL "Intel"  AND  UNIX))
    set(C_FLAGS "${CMAKE_C_FLAGS}")
    separate_arguments(C_FLAGS)
    set(SRCS main.s)
    add_custom_command(
      OUTPUT main.s
      COMMAND ${CMAKE_C_COMPILER} ${C_FLAGS} -S ${CMAKE_CURRENT_SOURCE_DIR}/main.c -o main.s
      DEPENDS main.c
      VERBATIM
      )
  endif(("${CMAKE_C_COMPILER_ID}" MATCHES "^(GNU|HP|SunPro|XL)$") OR ("${CMAKE_C_COMPILER_ID}" STREQUAL "Intel"  AND  UNIX))
endif("${CMAKE_GENERATOR}" MATCHES "Makefile")


if(SRCS)
  enable_language(ASM OPTIONAL)
else(SRCS)
  message(STATUS "No assembler enabled, using C")
  set(SRCS main.c)
endif(SRCS)

add_executable(HelloAsm ${SRCS})
