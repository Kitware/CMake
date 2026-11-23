enable_language(Fortran)

# Fortran modules are compile-time artifacts produced by compiling the
# dependency, so the weak ordering must not apply: the consumer keeps the
# full target-level ordering (old behavior).
add_library(fdep STATIC mod_provider.f90)
set_target_properties(fdep PROPERTIES
  Fortran_MODULE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/mod")
target_include_directories(fdep PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/mod")
add_executable(fmain mod_user.f90)
target_link_libraries(fmain PRIVATE fdep)
