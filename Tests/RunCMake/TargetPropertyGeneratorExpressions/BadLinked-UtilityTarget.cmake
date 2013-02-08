
add_custom_target(check ALL
  COMMAND ${CMAKE_COMMAND} -E echo check
)

add_library(foo STATIC empty.cpp)
set_property(TARGET foo PROPERTY INCLUDE_DIRECTORIES $<LINKED:check>)
