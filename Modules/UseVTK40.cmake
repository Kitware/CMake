#

# This is an implementation detail for using VTK 4.0 with the
# FindVTK.cmake module.  Do not include directly by name.  This should
# be included only when FindVTK.cmake sets the VTK_USE_FILE variable
# to point here.

# Load the compiler settings used for VTK.
IF(VTK_BUILD_SETTINGS_FILE)
  INCLUDE(CMakeImportBuildSettings)
  CMAKE_IMPORT_BUILD_SETTINGS(${VTK_BUILD_SETTINGS_FILE})
ENDIF(VTK_BUILD_SETTINGS_FILE)

# Add compiler flags needed to use VTK.
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VTK_REQUIRED_C_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${VTK_REQUIRED_CXX_FLAGS}")

# Add include directories needed to use VTK.
INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})

# Add link directories needed to use VTK.
LINK_DIRECTORIES(${VTK_LIBRARY_DIRS})
