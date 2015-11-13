cmake_minimum_required(VERSION 3.4)
enable_language(C)

add_library(Framework SHARED foo.c)
set_target_properties(Framework PROPERTIES FRAMEWORK TRUE)
