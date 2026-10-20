project(TransitiveDependencies LANGUAGES NONE)

add_library(root INTERFACE)
add_library(middle INTERFACE)
add_library(leaf INTERFACE)
add_library(side INTERFACE)
add_library(tail INTERFACE)

set_property(TARGET root PROPERTY SPDX_LICENSE MIT)
set_property(TARGET middle PROPERTY SPDX_LICENSE Apache-2.0)
set_property(TARGET leaf PROPERTY SPDX_LICENSE BSD-3-Clause)
set_property(TARGET side PROPERTY SPDX_LICENSE Zlib)
set_property(TARGET tail PROPERTY SPDX_LICENSE ISC)

target_link_libraries(root INTERFACE middle side)
target_link_libraries(middle INTERFACE leaf)
target_link_libraries(side INTERFACE leaf)
target_link_libraries(leaf INTERFACE tail)
target_link_libraries(tail INTERFACE middle)

install(TARGETS root EXPORT traversal DESTINATION .)
