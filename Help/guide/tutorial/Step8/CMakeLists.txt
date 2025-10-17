cmake_minimum_required(VERSION 3.23)

project(Tutorial)

option(TUTORIAL_BUILD_UTILITIES "Build the Tutorial executable" ON)
option(TUTORIAL_USE_STD_SQRT "Use std::sqrt" OFF)
option(TUTORIAL_ENABLE_IPO "Check for and use IPO support" ON)

# TODO6: Add a default-ON option named BUILD_TESTING with a doc string of:
#          "Enable testing and build tests"

if(TUTORIAL_ENABLE_IPO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT result OUTPUT output)
  if(result)
    message("IPO is supported, enabling IPO")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
  else()
    message(WARNING "IPO is not supported: ${output}")
  endif()
endif()

if(TUTORIAL_BUILD_UTILITIES)
  add_subdirectory(Tutorial)
endif()

# TODO7: Conditional on the value of BUILD_TESTING, enable testing and add the
#        Tests subdirectory to the project

add_subdirectory(MathFunctions)
