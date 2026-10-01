cmake_minimum_required(VERSION 3.10)
project(LinkDirectoryExternal C)

if(MSVC)
  # Simulate another language overriding the link directory flag.
  # The generators prefer CMAKE_<LANG>_LIBRARY_PATH_FLAG.
  set(CMAKE_LIBRARY_PATH_FLAG "bad-libpath-flag:")
endif()


set(cfg_dir)
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  set(cfg_dir /$<CONFIG>)
endif()

add_executable(myexe2 myexe.c)
set_property(TARGET myexe2 PROPERTY OUTPUT_NAME LinkDirectory2)
target_link_directories(myexe2 PRIVATE lib${cfg_dir} "${CMAKE_CURRENT_SOURCE_DIR}/../lib${cfg_dir}")
target_link_libraries(myexe2 PRIVATE mylibA mylibB)

add_library (mylibs INTERFACE)
target_link_directories(mylibs INTERFACE lib${cfg_dir} "${CMAKE_CURRENT_SOURCE_DIR}/../lib${cfg_dir}")
target_link_libraries(mylibs INTERFACE mylibA mylibB)
add_executable(myexe3 myexe.c)
set_property(TARGET myexe3 PROPERTY OUTPUT_NAME LinkDirectory3)
target_link_libraries(myexe3 PRIVATE mylibs)

link_directories(../lib${cfg_dir})
link_directories(lib${cfg_dir})

add_executable(myexe myexe.c)
set_property(TARGET myexe PROPERTY OUTPUT_NAME LinkDirectory)
target_link_libraries(myexe mylibA mylibB)
