cmake_minimum_required(VERSION 4.4)
project(empty_imported_configuration NONE)

add_library(imported UNKNOWN IMPORTED)
set_property(TARGET imported PROPERTY IMPORTED_CONFIGURATIONS "")

file(GENERATE OUTPUT out.txt CONTENT "$<TARGET_FILE:imported>")
