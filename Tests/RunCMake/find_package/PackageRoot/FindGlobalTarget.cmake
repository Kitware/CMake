add_library(imported_global_target SHARED IMPORTED GLOBAL)
add_executable(imported_global_ex IMPORTED GLOBAL)

add_library(imported_local_target SHARED IMPORTED)
add_executable(imported_local_ex IMPORTED)

find_package(SimpleTarget)
