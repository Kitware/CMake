#
# Find the native VTK includes and library
#
# This module defines:
#
# VTK_INSTALL_PATH  - Where is the installed version of VTK.
# USE_BUILT_VTK     - Should a built-from-source version of VTK be used.
#
# VTK_BINARY_PATH   - Where is (one of) the binary tree(s).
# USE_INSTALLED_VTK - Should an installed version of VTK be used
#
# USE_VTK_FILE      - (internal)
#                     Full path and location to the UseVTK.cmake file.
#                     This value changes each time USE_BUILT_VTK or 
#                     USE_INSTALLED_VTK is modified, and only if ONE of
#                     these setting is set to ON.
#
# If VTK is not found, an error is displayed unless VTK_FIND_QUIETLY
# is on.
#

#
# Look for a binary tree (built from source)
# 
FIND_PATH(VTK_BINARY_PATH UseVTK.cmake
  [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild1]
  [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild2]
  [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild3]
  [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild4]
  [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild5]
  [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild6]
  [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild7]
  [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild8]
  [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild9]
  [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild10]
  ../VTKBIN
  ../vtkbin
  VTKBIN
  vtkbin
  $ENV{HOME}/VTKBIN
  $ENV{HOME}/vtkbin
)

#
# If we found a built tree, USE_BUILT_VTK allows the user to 
# use it or not. Defaults to ON.
#
IF (VTK_BINARY_PATH)
  SET (USE_BUILT_VTK 1 CACHE BOOL 
       "Use a built (versus installed) version of VTK. Be sure that VTK_BINARY_PATH is correct.")
ENDIF (VTK_BINARY_PATH)

#
# Look for an installed tree
#
FIND_PATH(VTK_INSTALL_PATH include/vtk/UseVTK.cmake
  /usr/local
  /usr
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Kitware\\VTK\\Nightly]
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Kitware\\VTK\\43]
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Kitware\\VTK\\42]
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Kitware\\VTK\\41]
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Kitware\\VTK\\40]
)

#
# If we found an installed tree, USE_INSTALLED_VTK allows the user to 
# use it or not.
#
# Defaults to OFF if a built tree was found before (see above), ON otherwise.
#
IF (VTK_INSTALL_PATH)
  IF (USE_BUILT_VTK)
    SET (USE_INSTALLED_VTK 0 CACHE BOOL 
         "Use an installed (versus built from source) version of VTK. Be sure that VTK_INSTALL_PATH is correct.")
  ELSE (USE_BUILT_VTK)
    SET (USE_INSTALLED_VTK 1 CACHE BOOL 
         "Use an installed (versus built from source) version of VTK. Be sure that VTK_INSTALL_PATH is correct.")
  ENDIF (USE_BUILT_VTK)
ENDIF (VTK_INSTALL_PATH)

#
# Set the USE_VTK_FILE only if one of USE_BUILT_VTK or USE_INSTALLED_VTK
# is ON.
#
IF (USE_BUILT_VTK)
  IF (NOT USE_INSTALLED_VTK)
    IF (EXISTS "${VTK_BINARY_PATH}/UseVTK.cmake")
      SET (USE_VTK_FILE "${VTK_BINARY_PATH}/UseVTK.cmake")
    ENDIF (EXISTS "${VTK_BINARY_PATH}/UseVTK.cmake")
  ENDIF (NOT USE_INSTALLED_VTK)
ELSE (USE_BUILT_VTK)
  IF (USE_INSTALLED_VTK)
    IF (EXISTS "${VTK_INSTALL_PATH}/include/vtk/UseVTK.cmake")
      SET (USE_VTK_FILE "${VTK_INSTALL_PATH}/include/vtk/UseVTK.cmake")
    ENDIF (EXISTS "${VTK_INSTALL_PATH}/include/vtk/UseVTK.cmake")
  ENDIF (USE_INSTALLED_VTK)
ENDIF (USE_BUILT_VTK)

#
# Display a warning if both settings are set to ON.
# Otherwise display a warning if VTK was not found.
#
IF (USE_BUILT_VTK AND USE_INSTALLED_VTK)
  MESSAGE ("Warning. Please make sure that only ONE of the USE_INSTALLED_VTK or USE_BUILT_VTK setting is set to ON.")
ELSE (USE_BUILT_VTK AND USE_INSTALLED_VTK)
  IF (NOT USE_VTK_FILE)
    IF (NOT VTK_FIND_QUIETLY)
      MESSAGE ("Warning. VTK might be found on your system as different flavours: installed VTK or built VTK. Please make sure that the VTK_INSTALL_PATH or VTK_BINARY_PATH setting reflects which VTK you are planning to use, then set ONE of the USE_INSTALLED_VTK or USE_BUILT_VTK setting to ON.")
    ENDIF (NOT VTK_FIND_QUIETLY)
  ENDIF (NOT USE_VTK_FILE)
ENDIF (USE_BUILT_VTK AND USE_INSTALLED_VTK)

# Note:
#
# If you use that module then you are probably relying on VTK to be found
# on your system. Here is the CMake code you might probably want to use to
# work hand-in-hand with that module (in that example CAN_BUILD is a var
# that is set to 0 if your project can not be build):
#
# INCLUDE (${CMAKE_ROOT}/Modules/FindVTK.cmake)
#
# IF (USE_VTK_FILE)
#   INCLUDE (${USE_VTK_FILE})
# ELSE (USE_VTK_FILE)
#   SET (CAN_BUILD 0)
# ENDIF (USE_VTK_FILE)
