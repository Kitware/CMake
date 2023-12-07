add_subdirectory(subdir)

add_library(mvstos_lib "$<TARGET_OBJECTS:mvstos_obj>")
target_include_directories(mvstos_lib PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/subdir")
add_library(mvstos_use use.f90)
target_link_libraries(mvstos_use PRIVATE mvstos_lib)
