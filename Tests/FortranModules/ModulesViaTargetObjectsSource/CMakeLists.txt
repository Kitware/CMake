add_library(mvtos_obj OBJECT obj.f90)
add_library(mvtos_lib "$<TARGET_OBJECTS:mvtos_obj>")
add_library(mvtos_use use.f90)
target_link_libraries(mvtos_use PRIVATE mvtos_lib)
