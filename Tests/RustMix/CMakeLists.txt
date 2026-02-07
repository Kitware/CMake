cmake_minimum_required(VERSION 4.2)

set(CMAKE_EXPERIMENTAL_RUST "3cc9b32c-47d3-4056-8953-d74e69fc0d6c")

project(RustMix LANGUAGES C Rust)

add_library(liba STATIC liba.rs)
add_library(libb SHARED libb.rs)
add_library(libc OBJECT libc.rs)

add_executable(RustMix main.c)
target_link_libraries(RustMix liba)
target_link_libraries(RustMix libb)
target_link_libraries(RustMix libc)
