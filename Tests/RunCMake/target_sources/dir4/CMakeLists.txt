target_sources(lib1 PRIVATE FILE_SET HEADERS BASE_DIRS ${CMAKE_SOURCE_DIR} FILES
  $<1:dir3.h>
  dir4.h
  )
