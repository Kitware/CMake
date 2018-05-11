add_executable(subexe ../empty.c)
add_library(sublibUnknownImported UNKNOWN IMPORTED)

target_link_libraries(mainexe mainlibUnknownImported)
target_link_libraries(mainexe sublibUnknownImported)
get_property(mainexe_libs TARGET mainexe PROPERTY INTERFACE_LINK_LIBRARIES)
message(STATUS "mainexe: ${mainexe_libs}")
