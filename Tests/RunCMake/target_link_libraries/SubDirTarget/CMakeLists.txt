add_executable(subexe ../empty.c)
add_library(sublib ../empty.c)

target_link_libraries(mainexe mainlib)
target_link_libraries(mainexe sublib)
get_property(mainexe_libs TARGET mainexe PROPERTY INTERFACE_LINK_LIBRARIES)
message(STATUS "mainexe: ${mainexe_libs}")
