enable_language(C)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY out)

add_library(SharedTop SHARED mylib.c)
add_library(StaticTop STATIC mylib.c)
add_library(StaticMiddle STATIC mylib.c)

add_library(StaticNone STATIC mylib.c)
add_library(StaticPreBuild STATIC mylib.c)
add_library(StaticPreLink STATIC mylib.c)
add_library(StaticPostBuild STATIC mylib.c)
add_library(StaticCc STATIC mylibcc.c)

add_custom_command(TARGET StaticPreBuild PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E true)
add_custom_command(TARGET StaticPreLink PRE_LINK
  COMMAND ${CMAKE_COMMAND} -E true)
add_custom_command(TARGET StaticPostBuild POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E true)
add_custom_command(OUTPUT mylibcc.c
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/mylib.c ${CMAKE_BINARY_DIR}/mylibcc.c)

target_link_libraries(SharedTop PRIVATE StaticMiddle)
target_link_libraries(StaticTop PRIVATE StaticMiddle)
target_link_libraries(StaticMiddle PRIVATE StaticNone StaticPreBuild StaticPreLink StaticPostBuild StaticCc)

if(OPTIMIZE_TOP)
  set_target_properties(SharedTop StaticTop PROPERTIES
    OPTIMIZE_DEPENDENCIES TRUE)
endif()
if(OPTIMIZE_MIDDLE)
  set_target_properties(StaticMiddle PROPERTIES
    OPTIMIZE_DEPENDENCIES TRUE)
endif()

include(WriteTargets.cmake)
write_targets()
