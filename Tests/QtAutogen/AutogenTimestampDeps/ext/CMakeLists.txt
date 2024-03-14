cmake_minimum_required(VERSION 3.28)
project(Ext C)
add_library(ext STATIC ext.c)
set_property(TARGET ext PROPERTY ARCHIVE_OUTPUT_DIRECTORY "$<1:${CMAKE_CURRENT_BINARY_DIR}>")
