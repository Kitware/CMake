cmake_minimum_required(VERSION 3.4)
enable_language(C)

add_library(Framework SHARED
            foo.c
            foo.h
            res.txt)
set_target_properties(Framework PROPERTIES
                      FRAMEWORK TRUE
                      PUBLIC_HEADER foo.h
                      RESOURCE "res.txt")
