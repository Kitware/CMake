message(STATUS "CMAKE_XCODE_BUILD_SYSTEM='${CMAKE_XCODE_BUILD_SYSTEM}'")
if(CMAKE_GENERATOR_TOOLSET STREQUAL "Test Toolset")
  message(FATAL_ERROR "CMAKE_GENERATOR_TOOLSET is \"Test Toolset\" as expected.")
else()
  message(FATAL_ERROR
    "CMAKE_GENERATOR_TOOLSET is \"${CMAKE_GENERATOR_TOOLSET}\" "
    "but should be \"Test Toolset\"!")
endif()
