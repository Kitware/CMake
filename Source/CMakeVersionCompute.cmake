# Load version number components.
include(${CMake_SOURCE_DIR}/Source/CMakeVersion.cmake)

# Releases define a small tweak level.
if("${CMake_VERSION_TWEAK}" VERSION_LESS 20000000)
  set(CMake_VERSION_IS_RELEASE 1)
  set(CMake_VERSION_SOURCE "")
else()
  set(CMake_VERSION_IS_RELEASE 0)
  include(${CMake_SOURCE_DIR}/Source/CMakeVersionSource.cmake)
endif()

# Compute the full version string.
set(CMake_VERSION ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CMake_VERSION_PATCH})
if(${CMake_VERSION_TWEAK} GREATER 0)
  set(CMake_VERSION ${CMake_VERSION}.${CMake_VERSION_TWEAK})
endif()
if(CMake_VERSION_RC)
  set(CMake_VERSION ${CMake_VERSION}-rc${CMake_VERSION_RC})
endif()
if(CMake_VERSION_SOURCE)
  set(CMake_VERSION ${CMake_VERSION}-${CMake_VERSION_SOURCE})
endif()
