cmake_minimum_required(VERSION 3.15)
project(PchMultilanguage C CXX)

add_executable(foobar
    foo.c
    main.cpp
)
target_include_directories(foobar PUBLIC include)
target_precompile_headers(foobar PRIVATE "<stddef.h>")
