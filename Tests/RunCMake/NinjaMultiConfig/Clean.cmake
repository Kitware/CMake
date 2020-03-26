enable_language(C)

add_executable(exeall main.c)
set_target_properties(exeall PROPERTIES VERSION 1.0.0)
add_executable(exenotall main.c)
set_target_properties(exenotall PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_library(mylib SHARED simplelib.c)
set_target_properties(mylib PROPERTIES
  VERSION 1.0.0
  SOVERSION 1
  )

add_library(myobj OBJECT simplelib.c)

include(${CMAKE_CURRENT_LIST_DIR}/Common.cmake)
generate_output_files(exeall exenotall mylib myobj)
