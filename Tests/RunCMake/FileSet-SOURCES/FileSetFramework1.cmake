enable_language(C)

add_library(lib1 SHARED)
target_sources(lib1
  PUBLIC FILE_SET SOURCES FILES lib1.c
         FILE_SET HEADERS FILES lib1.h
  )
set_property(TARGET lib1 PROPERTY FRAMEWORK ON)
