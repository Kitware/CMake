add_subdirectory(subdir)

add_library(mvstol_lib dummy.f90)
target_link_libraries(mvstol_lib PRIVATE "$<TARGET_OBJECTS:mvstol_obj>")
target_include_directories(mvstol_lib PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/subdir")
add_library(mvstol_use use.f90)
target_link_libraries(mvstol_use PRIVATE mvstol_lib)
