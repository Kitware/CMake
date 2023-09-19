enable_language(C)

add_library(fortran_source_iface_sources STATIC lib.c)
target_sources(fortran_source_iface_sources
  INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/iface.f90")

add_library(lib25252-iface-sources lib.f90)
target_link_libraries(lib25252-iface-sources PRIVATE fortran_source_iface_sources)
