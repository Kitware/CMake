# Load version number components.
include(${CMake_SOURCE_DIR}/Source/CMakeVersion.cmake)

set(CMake_VERSION_IS_DIRTY 0)

# Our build definition sets this variable to control the patch part of the version number.
if(Microsoft_CMake_VERSION_PATCH)
  set(CMake_VERSION_PATCH ${Microsoft_CMake_VERSION_PATCH})
endif()

# Compute the full version string.
set(CMake_VERSION ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CMake_VERSION_PATCH}-MSVC_2)

# Compute the binary version that goes into the RC file. Each component of the RC version is 
# a 16-bit integer. Our patch version is of the form yymmddbb so we just split it in half.
string(SUBSTRING ${CMake_VERSION_PATCH} 0 4 CMake_RCVERSION_PATCH)
string(SUBSTRING ${CMake_VERSION_PATCH} 4 -1 CMake_RCVERSION_REV)

# The '8' is an identifier to indicate it comes from our Microsoft/CMake fork. It gets appended 
# at the end to ensure the revision component is still a 16-bit number.
set(CMake_RCVERSION ${CMake_VERSION_MAJOR},${CMake_VERSION_MINOR},${CMake_RCVERSION_PATCH},${CMake_RCVERSION_REV}8)
set(CMake_RCVERSION_STR ${CMake_VERSION})
