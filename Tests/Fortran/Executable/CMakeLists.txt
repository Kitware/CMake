include_directories(${Library_MODDIR})
include_directories(${testf_BINARY_DIR}/External)
link_directories(${testf_BINARY_DIR}/External)

add_executable(subdir_exe2 main.f90)
target_link_libraries(subdir_exe2 subdir_mods)
add_dependencies(subdir_exe2 ExternalTarget)
target_link_libraries(subdir_exe2 myext)
