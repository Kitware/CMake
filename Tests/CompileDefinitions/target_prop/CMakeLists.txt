
project(target_prop)

add_executable(target_prop_executable ../compiletest.cpp)

set_target_properties(target_prop_executable PROPERTIES COMPILE_DEFINITIONS CMAKE_IS_FUN)

set_property(TARGET target_prop_executable APPEND PROPERTY COMPILE_DEFINITIONS CMAKE_IS_REALLY="Very Fun" CMAKE_IS=Fun)
set_property(TARGET target_prop_executable APPEND PROPERTY COMPILE_DEFINITIONS CMAKE_IS_FUN CMAKE_IS_="Fun")
