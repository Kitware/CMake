cmake_minimum_required(VERSION 3.20.0)

project(subpro_c_nested_lib LANGUAGES NONE)

include(../Subproject.cmake)
add_subproject(c1_lib   DIR subsubpro_c1_lib)
add_subproject(c2_lib   DIR subsubpro_c2_lib
  DEPENDS
    c1_lib
)
