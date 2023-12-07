cmake_minimum_required(VERSION 3.12)
project(${RunCMake_TEST} C)

set(main_sources main.c)
set(library_sources library.c)

include(util.cmake)

include(${RunCMake_TEST}.cmake)
