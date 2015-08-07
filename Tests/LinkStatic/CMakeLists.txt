cmake_minimum_required(VERSION 2.8.4.20110303 FATAL_ERROR)
project(LinkStatic C)

if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
  message(FATAL_ERROR "This test works only with the GNU compiler!")
endif()

find_library(MATH_LIBRARY NAMES libm.a)
if(MATH_LIBRARY)
  get_filename_component(MATH_LIB_DIR ${MATH_LIBRARY} PATH)
  link_directories(${MATH_LIB_DIR})
  # Name the library both with a full path and as "-lm" to
  # activate the link type switching code for both cases.
  # If the second one links shared then the link will fail.
  set(MATH_LIBRARIES ${MATH_LIBRARY} -lm)
else()
  message(FATAL_ERROR "Cannot find libm.a needed for this test")
endif()

add_executable(LinkStatic LinkStatic.c)
target_link_libraries(LinkStatic ${MATH_LIBRARIES})

# Enable static linking.
set(LinkStatic_FLAG "-static" CACHE STRING "Flag to link statically")
set_property(TARGET LinkStatic PROPERTY LINK_FLAGS "${LinkStatic_FLAG}")
set_property(TARGET LinkStatic PROPERTY LINK_SEARCH_START_STATIC 1)
#set_property(TARGET LinkStatic PROPERTY LINK_SEARCH_END_STATIC 1) # insufficient

set(CMAKE_LINK_SEARCH_START_STATIC ON)
add_executable(LinkStaticInitStart1 LinkStatic.c)
get_property(LSSS TARGET LinkStaticInitStart1
  PROPERTY LINK_SEARCH_START_STATIC)
if(NOT LSSS)
  message(FATAL_ERROR "Failed to correctly initialize LINK_SEARCH_START_STATIC")
endif()

set(CMAKE_LINK_SEARCH_START_STATIC OFF)
add_executable(LinkStaticInitStart2 LinkStatic.c)
get_property(LSSS TARGET LinkStaticInitStart2
  PROPERTY LINK_SEARCH_START_STATIC)
if(LSSS)
  message(FATAL_ERROR "Failed to correctly initialize LINK_SEARCH_START_STATIC")
endif()

unset(CMAKE_LINK_SEARCH_START_STATIC)

set(CMAKE_LINK_SEARCH_END_STATIC ON)
add_executable(LinkStaticInitEnd1 LinkStatic.c)
get_property(LSES TARGET LinkStaticInitEnd1
  PROPERTY LINK_SEARCH_END_STATIC)
if(NOT LSES)
  message(FATAL_ERROR "Failed to correctly initialize LINK_SEARCH_END_STATIC")
endif()

set(CMAKE_LINK_SEARCH_END_STATIC OFF)
add_executable(LinkStaticInitEnd2 LinkStatic.c)
get_property(LSES TARGET LinkStaticInitEnd2
  PROPERTY LINK_SEARCH_END_STATIC)
if(LSES)
  message(FATAL_ERROR "Failed to correctly initialize LINK_SEARCH_END_STATIC")
endif()

unset(CMAKE_LINK_SEARCH_END_STATIC)
