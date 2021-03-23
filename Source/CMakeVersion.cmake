# CMake version number components.
set(CMake_VERSION_MAJOR 3)
set(CMake_VERSION_MINOR 20)
set(CMake_VERSION_PATCH 0)
#set(CMake_VERSION_RC 0)
set(CMake_VERSION_IS_DIRTY 0)

# Our build definition sets this variable to control the patch part of the version number.
if(Microsoft_CMake_VERSION_PATCH)
  set(CMake_VERSION_PATCH ${Microsoft_CMake_VERSION_PATCH})
endif()

# Compute the full version string.
set(CMake_VERSION ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CMake_VERSION_PATCH}-MSVC_2)

# Compute the binary version that goes into the RC file.
# CMake version has the format major.minor.patch[-suffix] but for RC files
# we need to split patch into two components because each component is a
# 16-bit integer. Our patch numbers have the format yymmddbb so we split
# that in half and append an "8" to identify a build coming from our branch.
#
# Example for build 02 generated on 12/1/2017
# cmake version 3.10.17120102-MSVC_2
# binary version that appears in file properties 3.10.1712.01028
#
# The reason we need consistency is for Watson crash dumps. It will report
# the binary file version and we need to match it to our build.

# Each component of the RC version is a 16-bit integer. Our patch version
# is of the form yymmddbb so we just split it in half.
string(SUBSTRING ${CMake_VERSION_PATCH} 0 4 CMake_RCVERSION_PATCH)
string(SUBSTRING ${CMake_VERSION_PATCH} 4 -1 CMake_RCVERSION_REV)

# The '8' is an identifier to indicate it comes from our Microsoft/CMake fork. It gets appended
# at the end to ensure the revision component is still a 16-bit number.
set(CMake_RCVERSION ${CMake_VERSION_MAJOR},${CMake_VERSION_MINOR},${CMake_RCVERSION_PATCH},${CMake_RCVERSION_REV}8)
set(CMake_RCVERSION_STR ${CMake_VERSION})
