enable_language(C)

add_library(fortran_target_objects_sources STATIC "${CMAKE_CURRENT_SOURCE_DIR}/iface.f90")

add_library(lib25365-target-objects lib.f90 "$<TARGET_OBJECTS:fortran_target_objects_sources>")
