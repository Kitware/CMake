cmake_minimum_required(VERSION 4.2)

set(CMAKE_EXPERIMENTAL_RUST "628f8f9f-d7dc-4a06-9aaf-d4ede4426ddf")

enable_language(Rust)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
add_library(main OBJECT main.rs)
