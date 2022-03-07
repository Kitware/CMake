
cmake_minimum_required(VERSION 3.23)

project(External LANGUAGES C)

add_library(external SHARED ../unref.c)
