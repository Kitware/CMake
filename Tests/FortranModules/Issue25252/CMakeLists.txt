add_custom_target(custom_target_with_fortran
  COMMAND "${CMAKE_COMMAND}" -E echo "custom target with fortran sources"
  SOURCES custom_target.f90)

add_library(lib25252 lib.f90)
add_dependencies(lib25252 custom_target_with_fortran)
