cmake_minimum_required(VERSION 3.23)

project(Tutorial)

option(TUTORIAL_BUILD_UTILITIES "Build the Tutorial executable" ON)

# TODO1: Add a default-OFF option named TUTORIAL_USE_STD_SQRT, with a doc
#        string of "Use std::sqrt"

if(TUTORIAL_BUILD_UTILITIES)
  add_subdirectory(Tutorial)
endif()

add_subdirectory(MathFunctions)
add_subdirectory(Vendor)
