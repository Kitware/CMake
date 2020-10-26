message(STATUS "CMAKE_XCODE_BUILD_SYSTEM='${CMAKE_XCODE_BUILD_SYSTEM}'")
if(CMAKE_GENERATOR_TOOLSET STREQUAL "Test Toolset,buildsystem=12")
  message(FATAL_ERROR "CMAKE_GENERATOR_TOOLSET is \"Test Toolset,buildsystem=12\" as expected.")
else()
  message(FATAL_ERROR
    "CMAKE_GENERATOR_TOOLSET is \"${CMAKE_GENERATOR_TOOLSET}\" "
    "but should be \"Test Toolset,buildsystem=12\"!")
endif()
