cmake_minimum_required(VERSION 3.28)

project(NasmOnly LANGUAGES ASM_NASM)

add_library(testnasm1 STATIC libnasm1.nasm)
add_executable(NasmOnly nasmonly.nasm)
target_link_libraries(NasmOnly testnasm1)
