cmake_minimum_required(VERSION 2.8.1 FATAL_ERROR)
project(CheckTypeSize C)

include(CheckTypeSize)
check_type_size("void*"     SIZEOF_DATA_PTR)
check_type_size(char        SIZEOF_CHAR)
check_type_size(short       SIZEOF_SHORT)
check_type_size(int         SIZEOF_INT)
check_type_size(long        SIZEOF_LONG)
check_type_size("long long" SIZEOF_LONG_LONG)
check_type_size(__int64     SIZEOF___INT64)
check_type_size(size_t      SIZEOF_SIZE_T)
check_type_size(ssize_t     SIZEOF_SSIZE_T)

configure_file(config.h.in config.h)
include_directories(${CheckTypeSize_BINARY_DIR})

add_executable(CheckTypeSize CheckTypeSize.c)
