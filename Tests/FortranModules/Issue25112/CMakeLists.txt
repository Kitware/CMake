set(CMAKE_Fortran_MODULE_DIRECTORY "${PROJECT_BINARY_DIR}/include")
add_library(objmod OBJECT objmod.f90)
add_executable(objmain objmain.f90)
target_link_libraries(objmain PRIVATE objmod)
