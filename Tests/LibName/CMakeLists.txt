project(LibName)
# this is a test to make sure that relative path 
# LIBRARY_OUTPUT_PATH and EXECUTABLE_OUTPUT_PATH work
set(LIBRARY_OUTPUT_PATH lib)
set(EXECUTABLE_OUTPUT_PATH lib)
add_library(bar SHARED bar.c)
add_library(foo SHARED foo.c)
target_link_libraries(foo bar)
add_executable(foobar foobar.c)
target_link_libraries(foobar foo)
IF(UNIX)
  target_link_libraries(foobar -L/usr/local/lib)
ENDIF(UNIX)
