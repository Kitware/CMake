enable_language(C)

add_library(fortran_target_objects_sources_iface STATIC "${CMAKE_CURRENT_SOURCE_DIR}/iface.f90")

add_library(fortran_target_objects_sources_iface_bridge INTERFACE)
target_sources(fortran_target_objects_sources_iface_bridge
  INTERFACE
    "$<TARGET_OBJECTS:fortran_target_objects_sources_iface>")

add_library(lib25365-target-objects-iface lib.f90)
target_link_libraries(lib25365-target-objects-iface PRIVATE fortran_target_objects_sources_iface_bridge)
