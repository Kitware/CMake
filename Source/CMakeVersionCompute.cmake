# Load version number components.
include(${CMake_SOURCE_DIR}/Source/CMakeVersion.cmake)

set(CMake_VERSION_IS_DIRTY 0)

# Our build definition sets this variable to control the patch part of the version number.
if(Microsoft_CMake_VERSION_PATCH)
  set(CMake_VERSION_PATCH ${Microsoft_CMake_VERSION_PATCH})
endif()

# Compute the full version string.
set(CMake_VERSION ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CMake_VERSION_PATCH}-MSVC_2)
