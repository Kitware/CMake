enable_language(C)

add_library(lib1 SHARED lib1.c)
set_property(TARGET lib1 PROPERTY FRAMEWORK ON)
target_sources(lib1
  PUBLIC FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} FILES h1.h
  )
