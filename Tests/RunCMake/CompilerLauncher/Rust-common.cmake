cmake_minimum_required(VERSION 4.2)

set(CMAKE_EXPERIMENTAL_RUST "b6fdddce-bf66-41a5-bc5f-077f6fa4d2a1")

enable_language(Rust)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
add_library(main OBJECT main.rs)
