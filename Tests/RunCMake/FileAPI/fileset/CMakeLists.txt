add_library(c_headers_1 STATIC empty.c)

target_sources(c_headers_1
  PUBLIC FILE_SET HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES error.c other.c
  PRIVATE FILE_SET a TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES h1.h
  )
target_sources(c_headers_1
  PUBLIC FILE_SET b TYPE HEADERS BASE_DIRS "$<1:${CMAKE_CURRENT_SOURCE_DIR}/dir>" FILES "$<1:${CMAKE_CURRENT_SOURCE_DIR}/dir/h2.h>"
  )
target_sources(c_headers_1
  INTERFACE FILE_SET c TYPE HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES h3.h
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
  )
install(TARGETS c_headers_2)
