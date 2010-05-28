cmake_minimum_required(VERSION 2.8)
project(LinkFlags C)

string(TOUPPER "${TEST_CONFIG}" TEST_CONFIG_UPPER)

add_library(LinkFlags_lib STATIC LinkFlagsLib.c)
set_property(TARGET LinkFlags_lib PROPERTY STATIC_LIBRARY_FLAGS _BADFLAG_)

add_library(LinkFlags_dll SHARED LinkFlagsLib.c)
set_property(TARGET LinkFlags_dll PROPERTY LINK_FLAGS _BADFLAG_)

add_executable(LinkFlags_exe LinkFlags.c)
set_property(TARGET LinkFlags_exe PROPERTY LINK_FLAGS _BADFLAG_)

add_library(LinkFlags_lib_config STATIC LinkFlagsLib.c)
set_property(TARGET LinkFlags_lib_config PROPERTY STATIC_LIBRARY_FLAGS_${TEST_CONFIG_UPPER} _BADFLAG_${TEST_CONFIG}_)

add_library(LinkFlags_dll_config SHARED LinkFlagsLib.c)
set_property(TARGET LinkFlags_dll_config PROPERTY LINK_FLAGS_${TEST_CONFIG_UPPER} _BADFLAG_${TEST_CONFIG}_)

add_executable(LinkFlags_exe_config LinkFlags.c)
set_property(TARGET LinkFlags_exe_config PROPERTY LINK_FLAGS_${TEST_CONFIG_UPPER} _BADFLAG_${TEST_CONFIG}_)

add_executable(LinkFlags LinkFlags.c)
