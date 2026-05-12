cmake_minimum_required(VERSION 4.3)
project(PellesASM LANGUAGES ASM_POASM)

string(TOLOWER "$ENV{LIB}" env_lib)
if(env_lib MATCHES [[[\/]+lib[\/]+win64[\/]*(;|$)]])
  set(arch "AMD64")
elseif(env_lib MATCHES [[[\/]+lib[\/]+win[\/]*(;|$)]])
  set(arch "IA32")
else()
  message(FATAL_ERROR "Cannot determine target architecture.")
endif()

add_executable(PellesASM main-${arch}.asm)
