add_library(fortran_source_iface INTERFACE
  iface.f90)

add_library(lib25252-iface-target lib.f90)
add_dependencies(lib25252-iface-target fortran_source_iface)
