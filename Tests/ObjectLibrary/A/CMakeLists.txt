project(ObjectLibraryA C)
# Add -fPIC so objects can be used in shared libraries.
# TODO: Need property for this.
if(CMAKE_SHARED_LIBRARY_C_FLAGS AND NOT WATCOM)
  set(CMAKE_C_FLAGS "${CMAKE_SHARED_LIBRARY_C_FLAGS} ${CMAKE_C_FLAGS}")
endif()

add_definitions(-DA_DEF)

add_custom_command(
  OUTPUT a1.c
  DEPENDS a1.c.in
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/a1.c.in
                                   ${CMAKE_CURRENT_BINARY_DIR}/a1.c
  )

add_library(A OBJECT a1.c a2.c)
target_include_directories(A PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
