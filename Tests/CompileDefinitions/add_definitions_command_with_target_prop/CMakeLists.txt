
project(add_definitions_command_with_target_prop)

add_definitions(-DCMAKE_IS_FUN -DCMAKE_IS=Fun)

add_executable(add_definitions_command_with_target_prop_executable ../compiletest.cpp)

set_target_properties(add_definitions_command_with_target_prop_executable PROPERTIES COMPILE_DEFINITIONS CMAKE_IS_="Fun")

set_property(TARGET add_definitions_command_with_target_prop_executable APPEND PROPERTY COMPILE_DEFINITIONS CMAKE_IS_REALLY="Very Fun")

add_definitions(-DCMAKE_IS_FUN)

set_property(TARGET add_definitions_command_with_target_prop_executable APPEND PROPERTY COMPILE_DEFINITIONS CMAKE_IS=Fun CMAKE_IS_="Fun")

add_definitions(-DBUILD_IS_DEBUG=$<CONFIG:Debug>)
set_property(TARGET add_definitions_command_with_target_prop_executable APPEND PROPERTY COMPILE_DEFINITIONS BUILD_IS_NOT_DEBUG=$<NOT:$<CONFIG:Debug>>)
