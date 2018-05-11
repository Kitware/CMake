enable_language(C)

add_executable(mainexe empty.c)
add_library(mainlibUnknownImportedGlobal UNKNOWN IMPORTED GLOBAL)

add_subdirectory(SubDirTarget-UNKNOWN-IMPORTED-GLOBAL)

target_link_libraries(subexe mainlibUnknownImportedGlobal)
target_link_libraries(subexe sublibUnknownImportedGlobal)
get_property(subexe_libs TARGET subexe PROPERTY INTERFACE_LINK_LIBRARIES)
message(STATUS "subexe: ${subexe_libs}")
