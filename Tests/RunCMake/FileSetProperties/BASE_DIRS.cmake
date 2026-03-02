
enable_language(C)

add_library(foo STATIC foo.c)


target_sources(foo PUBLIC FILE_SET HEADERS FILES foo.h)

get_property(dirs FILE_SET HEADERS TARGET foo PROPERTY BASE_DIRS)
if(NOT dirs STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  message(SEND_ERROR "wrong base dirs: '${dirs}' instead of '${CMAKE_CURRENT_SOURCE_DIR}'")
endif()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/foo.h" "")
set_property(FILE_SET HEADERS TARGET foo PROPERTY BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}")
set_property(FILE_SET HEADERS TARGET foo PROPERTY SOURCES "${CMAKE_CURRENT_BINARY_DIR}/foo.h")

get_property(dirs FILE_SET HEADERS TARGET foo PROPERTY BASE_DIRS)
if(NOT dirs STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  message(SEND_ERROR "wrong base dirs: '${dirs}' instead of '${CMAKE_CURRENT_BINARY_DIR}'")
endif()
