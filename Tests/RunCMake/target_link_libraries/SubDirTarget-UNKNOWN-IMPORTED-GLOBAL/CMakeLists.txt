add_executable(subexe ../empty.c)
add_library(sublibUnknownImportedGlobal UNKNOWN IMPORTED GLOBAL)

target_link_libraries(mainexe mainlibUnknownImportedGlobal)
target_link_libraries(mainexe sublibUnknownImportedGlobal)
get_property(mainexe_libs TARGET mainexe PROPERTY INTERFACE_LINK_LIBRARIES)
message(STATUS "mainexe: ${mainexe_libs}")
