enable_language(C)

add_executable(mainexe empty.c)
add_library(mainlib empty.c)

add_subdirectory(SubDirTarget)

target_link_libraries(subexe mainlib)
target_link_libraries(subexe sublib)
get_property(subexe_libs TARGET subexe PROPERTY INTERFACE_LINK_LIBRARIES)
message(STATUS "subexe: ${subexe_libs}")
