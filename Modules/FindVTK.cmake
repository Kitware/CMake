#
# Find the native VTK includes and library
#
# This module defines
#
# VTK_INSTALL_PATH - where is the installed version of VTK
# VTK_BINARY_PATH - where is the binary tree (only defined if SOURCE_PATH is defined)
# USE_INSTALLED_VTK - sould an installed or source version of VTK be used
# USE_VTK_FILE - the full path and location of the UseVTK.cmake file
#

#
# Look for a binary tree
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
# If we found a binary tree then set use_installed_vtk to 0
#
IF (VTK_BINARY_PATH)
  SET (USE_INSTALLED_VTK 0 CACHE BOOL "Is an installed (versus source) version of VTK used")
ELSE (VTK_BINARY_PATH)
  # look for installed path
  FIND_PATH(VTK_INSTALL_PATH include/vtk/UseVTK.cmake
    /usr/local
    /usr
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Kitware\\VTK\\Nightly]
    )
  IF (VTK_INSTALL_PATH)
    SET (USE_INSTALLED_VTK 1 CACHE BOOL "Is an installed (versus source) version of VTK used")
  ENDIF (VTK_INSTALL_PATH)
ENDIF (VTK_BINARY_PATH)

IF (USE_INSTALLED_VTK)
  # look for the vtk header files in installed places
  FIND_PATH(VTK_INSTALL_PATH include/vtk/UseVTK.cmake
    /usr/local
    /usr
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Kitware\\VTK\\Nightly]
    )
  IF (VTK_INSTALL_PATH)
    SET (USE_INSTALLED_VTK 1 CACHE BOOL "Is an installed (versus source) version of VTK used")
  ENDIF (VTK_INSTALL_PATH)
ENDIF (USE_INSTALLED_VTK)


IF (USE_INSTALLED_VTK)
  IF (VTK_INSTALL_PATH)
    SET (USE_VTK_FILE ${VTK_INSTALL_PATH}/include/vtk/UseVTK.cmake)
  ENDIF (VTK_INSTALL_PATH)
ELSE (USE_INSTALLED_VTK)
  IF (VTK_BINARY_PATH)
    SET (USE_VTK_FILE ${VTK_BINARY_PATH}/UseVTK.cmake)
  ENDIF (VTK_BINARY_PATH)
ENDIF (USE_INSTALLED_VTK)



