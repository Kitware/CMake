find_package(OpenMP REQUIRED COMPONENTS C CXX)

add_executable(exe main.c)
target_link_libraries(exe PRIVATE OpenMP::OpenMP_C_Offload)

add_library(lib STATIC lib.c)
target_link_libraries(lib PRIVATE OpenMP::OpenMP_C_Offload)
add_executable(static main.c)
target_link_libraries(static PRIVATE lib)
add_executable(mixed main.cxx)
target_link_libraries(mixed PRIVATE lib)

# Not opted in, so these get no offload flags.
add_executable(hostc main.c)
target_link_libraries(hostc PRIVATE OpenMP::OpenMP_C)
add_executable(host main.cxx)
target_link_libraries(host PRIVATE OpenMP::OpenMP_CXX)
