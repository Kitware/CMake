#
# Find the native VTK includes and library
#
# This module defines:
#
# VTK_INSTALL_PATH  - where is the installed version of VTK.
# USE_BUILT_VTK     - should a built-from-source version of VTK be used
#
# VTK_BINARY_PATH   - where is (one of) the binary tree(s).
# USE_INSTALLED_VTK - should an installed version of VTK be used
#
# USE_VTK_FILE      - the full path and location of the UseVTK.cmake file
#                     ONLY SET if ONE of the USE_BUILT_VTK or 
#                     USE_INSTALLED_VTK is set to ON.
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
# If we found a built tree, USE_BUILT_VTK allows the user to 
# use it or not.
#
# Important: it *has* to be set to OFF first in order to allow
# the user to change the VTK_BINARY_PATH value *before* the USE_VTK_FILE
# is set (and the cache created). 
#
# For example, CMake could have picked-up the wrong VTK binary path, and if 
# the related VTK cache was immediately loaded there would not be any way
# to overwrite these cache values (even by changing the binary path or
# USE_VTK_FILE).
#
IF (VTK_BINARY_PATH)
  SET (USE_BUILT_VTK 0 CACHE BOOL 
       "Use a built (versus installed) version of VTK. Be sure VTK_BINARY_PATH is correct before setting it to ON.")
ENDIF (VTK_BINARY_PATH)

#
# If we found an installed tree, USE_INSTALLED_VTK allows the user to 
# use it or not.
#
# Important: it *has* to be set to OFF first in order to allow
# the user to change the VTK_INSTALL_PATH value before the USE_VTK_FILE
# is set (and the cache created).
#
# For example, you might have simultaneously installed different major
# versions like VTK 3.2, 4.0, 5.1, etc.
#
# Moreover, USE_INSTALLED_VTK has to be OFF since USE_BUILT_VTK has to be OFF
# too (see above). If USE_INSTALLED_VTK was ON by default, then USE_VTK_FILE
# would be set immediately (since one of them is ON) in favor of the 
# installed tree.
#
IF (VTK_INSTALL_PATH)
  SET (USE_INSTALLED_VTK 0 CACHE BOOL 
       "Use an installed (versus built from source) version of VTK. Be sure VTK_INSTALL_PATH is correct before setting it to ON.")
ENDIF (VTK_INSTALL_PATH)

#
# Set the USE_VTK_FILE only if one of USE_BUILT_VTK or USE_INSTALLED_VTK
# is ON, i.e. if the user has decided if he wants to use either the built
# or the installed VTK. Even if only one of the VTK flavor was found,
# this also enable the user to change the path to his VTK (in case the
# wrong installed or built VTK was automatically found).
#
# Once this decision has been made, there is no way to go back except by
# erasing the cache. Mark these useless vars as ADVANCED to reflect this.
#
IF (USE_BUILT_VTK)
  IF (NOT USE_INSTALLED_VTK)
    FIND_FILE(USE_VTK_FILE UseVTK.cmake ${VTK_BINARY_PATH})
    MARK_AS_ADVANCED(
      USE_BUILT_VTK
      USE_INSTALLED_VTK
      USE_VTK_FILE 
      VTK_INSTALL_PATH
    )
  ENDIF (NOT USE_INSTALLED_VTK)
ELSE (USE_BUILT_VTK)
  IF (USE_INSTALLED_VTK)
    SET (USE_VTK_FILE ${VTK_INSTALL_PATH}/include/vtk/UseVTK.cmake)
    MARK_AS_ADVANCED(
      USE_BUILT_VTK
      USE_INSTALLED_VTK
      USE_VTK_FILE 
      VTK_BINARY_PATH
    )
  ENDIF (USE_INSTALLED_VTK)
ENDIF (USE_BUILT_VTK)

# Note:
#
# If you use that module then you are probably relying on VTK to be found
# on your system. As said before, it might be found as different flavours: 
# installed VTK, or (multiple) built VTK. Moreover, even if it is found
# automatically, CMake might have picked the wrong path/version.
#
# Here is the CMake code you might probably want to use to work hand-in-hand
# with that module:
#
# INCLUDE (${CMAKE_ROOT}/Modules/FindVTK.cmake)
#
# IF (USE_VTK_FILE)
#   INCLUDE (${USE_VTK_FILE})
# ELSE (USE_VTK_FILE)
#   MESSAGE("Warning. This project is supposed to work with VTK, which might be found on your system as different flavours: installed VTK, or (multiple) built VTK. Please, make sure that the VTK_INSTALL_PATH or VTK_BINARY_PATH setting reflect which VTK you are planning to use, then set one of the USE_INSTALLED_VTK or USE_BUILT_VTK to ON.")
#   SET (CAN_BUILD 0)
# ENDIF (USE_VTK_FILE)
