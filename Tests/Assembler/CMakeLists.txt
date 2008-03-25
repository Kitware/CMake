cmake_minimum_required (VERSION 2.6)
project(Assembler)

set(SRCS)

# if no file has been set as source and we are on linux with an x86 processor try to use the gas/as assembler
# main-linux-x86-gas.s seems to work for Linux and FreeBSD
if(NOT SRCS AND CMAKE_SYSTEM_PROCESSOR MATCHES "[ix].?86$")
  if(CMAKE_SYSTEM MATCHES Linux OR CMAKE_SYSTEM MATCHES FreeBSD)
    message(STATUS "Trying to enable ASM-ATT for Linux or FreeBSD on x86")
    enable_language(ASM-ATT OPTIONAL)
    if(CMAKE_ASM-ATT_COMPILER_WORKS)
      message(STATUS "Trying to enable ASM-ATT for Linux/x86 - succeeded")
      # this assembler file was created using gcc -S main.c
      set(SRCS main-linux-x86-gas.s)
    endif(CMAKE_ASM-ATT_COMPILER_WORKS)
  endif(CMAKE_SYSTEM MATCHES Linux OR CMAKE_SYSTEM MATCHES FreeBSD)
endif(NOT SRCS AND CMAKE_SYSTEM_PROCESSOR MATCHES "[ix].?86$")

if(NOT SRCS)
  message(STATUS "No assembler enabled, using C")
  set(SRCS main.c)
endif(NOT SRCS)

add_executable(HelloAsm ${SRCS})
set_target_properties(HelloAsm PROPERTIES LINKER_LANGUAGE C)
