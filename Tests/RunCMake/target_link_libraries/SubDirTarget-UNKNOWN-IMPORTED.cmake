enable_language(C)

add_executable(mainexe empty.c)
add_library(mainlibUnknownImported UNKNOWN IMPORTED)

add_subdirectory(SubDirTarget-UNKNOWN-IMPORTED)

target_link_libraries(subexe mainlibUnknownImported)
target_link_libraries(subexe sublibUnknownImported)
get_property(subexe_libs TARGET subexe PROPERTY INTERFACE_LINK_LIBRARIES)
message(STATUS "subexe: ${subexe_libs}")
