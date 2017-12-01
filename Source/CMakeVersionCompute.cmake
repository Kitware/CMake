# Load version number components.
include(${CMake_SOURCE_DIR}/Source/CMakeVersion.cmake)

set(CMake_VERSION_IS_DIRTY 0)

# Our build definition sets this variable to control the patch part of the version number.
if(Microsoft_CMake_VERSION_PATCH)
  set(CMake_VERSION_PATCH ${Microsoft_CMake_VERSION_PATCH})
endif()

# Split the patch component because each version component in the RC file is a 16-bit integer.
# Our patch version is of the form yymmddbb so we just split it in half.
string(SUBSTRING ${CMake_VERSION_PATCH} 4 -1 CMake_VERSION_REV)
string(SUBSTRING ${CMake_VERSION_PATCH} 0 4 CMake_VERSION_PATCH)

# The '8' is an identifier to indicate it comes from our Microsoft/CMake fork.
set(CMake_RCVERSION ${CMake_VERSION_MAJOR},${CMake_VERSION_MINOR},${CMake_VERSION_PATCH},${CMake_VERSION_REV}8)
set(CMake_RCVERSION_STR ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CMake_VERSION_PATCH}.${CMake_VERSION_REV}8)

set(CMake_VERSION ${CMake_RCVERSION_STR})
