# See https://gist.github.com/scivision/8e3070319f0577f7d3efcba863638cae
set(CMAKE_Fortran_MODULE_DIRECTORY "${PROJECT_BINARY_DIR}/include")
add_library(m1 OBJECT m1.f90)

add_library(m2 OBJECT m2.f90)
target_link_libraries(m2 PRIVATE m1)

add_library(m3 OBJECT m3.f90)
target_link_libraries(m3 PRIVATE m2)

add_library(m4 OBJECT m4.f90)
target_link_libraries(m4 PRIVATE m3)

add_executable(main25223 main.f90)
target_link_libraries(main25223 PRIVATE m4 m3 m2 m1)
