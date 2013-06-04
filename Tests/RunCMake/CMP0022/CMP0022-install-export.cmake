
project(cmp0022NEW)

cmake_policy(SET CMP0022 NEW)

add_library(cmp0022NEW SHARED empty.cpp)
add_library(testLib SHARED empty.cpp)

set_property(TARGET cmp0022NEW APPEND PROPERTY LINK_INTERFACE_LIBRARIES testLib)

install(TARGETS cmp0022NEW testLib EXPORT exp DESTINATION lib)
install(EXPORT exp FILE expTargets.cmake DESTINATION lib/cmake/exp)
