add_library(c_headers_1 STATIC empty.c)

target_sources(c_headers_1
  PUBLIC FILE_SET HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES error.c other.c
  PRIVATE FILE_SET a TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES h1.h dir/h4.h h5.h
  )
target_sources(c_headers_1
  PUBLIC FILE_SET b TYPE HEADERS BASE_DIRS "$<1:${CMAKE_CURRENT_SOURCE_DIR}/dir>" FILES "$<1:${CMAKE_CURRENT_SOURCE_DIR}/dir/h2.h>"
  )
target_sources(c_headers_1
  INTERFACE FILE_SET c TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES h3.h "$<1:h5.h>"
  )
source_group("Source Files" FILES "${CMAKE_CURRENT_SOURCE_DIR}/other.c")

add_library(c_headers_2 STATIC empty.c)
target_sources(c_headers_2
  INTERFACE FILE_SET HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES h1.h
  )

install(TARGETS c_headers_1
  FILE_SET HEADERS DESTINATION include COMPONENT Headers
  FILE_SET b DESTINATION include/dir
  FILE_SET c

  # Non-extant FILE_SET should be ignored
  FILE_SET d
  )
install(TARGETS c_headers_2)

#
# SOURCES file set type
#
add_library(c_sources_1 STATIC)

target_sources(c_sources_1
  PRIVATE FILE_SET SOURCES BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES other.c
  )
set_property(FILE_SET SOURCES TARGET c_sources_1 PROPERTY COMPILE_DEFINITIONS DEF1)

target_sources(c_sources_1
  PUBLIC FILE_SET public_srcs TYPE SOURCES BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES empty.c
  )
set_property(FILE_SET public_srcs TARGET c_sources_1 PROPERTY COMPILE_DEFINITIONS DEF2)
set_property(FILE_SET public_srcs TARGET c_sources_1 PROPERTY INTERFACE_COMPILE_DEFINITIONS DEF3)

set_property(FILE_SET public_srcs TARGET c_sources_1 PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/dir")
set_property(FILE_SET public_srcs TARGET c_sources_1 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/dir")

add_library(c_sources_2 STATIC)
target_link_libraries(c_sources_2 PRIVATE c_sources_1)
