cmake_minimum_required(VERSION 3.23)

project(Tutorial)

option(TUTORIAL_BUILD_UTILITIES "Build the Tutorial executable" ON)
option(TUTORIAL_USE_STD_SQRT "Use std::sqrt" OFF)

# TODO6: Add a default-ON option named TUTORIAL_ENABLE_IPO with a doc string:
#          "Check for and use IPO support"

# TODO7: Include and use the CheckIPOSupported module to enable IPO if
#        TUTORIAL_ENABLE_IPO is True. To enable IPO, use:
#          set(CMAKE_INTERPROCEDURAL_OPTIMIZATION True)
#        Otherwise, follow the examples in the CheckIPOSupported documentation.
#        Specifically, follow the documentation example to emit an error message
#        if IPO is unavailable. Additionally, when IPO is available, emit a
#        message indicating so (otherwise IPO has no easily visible change in
#        the logs or build).

if(TUTORIAL_BUILD_UTILITIES)
  add_subdirectory(Tutorial)
endif()

add_subdirectory(MathFunctions)
