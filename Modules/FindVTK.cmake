#
# Find the native VTK includes and library
#
# This module defines
#
# VTK_INSTALL_DIR - where is the installed version of VTK
# VTK_SOURCE_DIR - where is VTK source code
# VTK_BINARY_DIR - where is the binary tree (only defined if SOURCE_PATH is defined)
# USE_INSTALLED_VTK - sould an installed or source version of VTK be used
#


#
# Look for local source trees and builds of VTK
#
# look in the cmake list of recent source and bin dirs for this user
#
FIND_PATH(VTK_SOURCE_DIR Common/vtkObject.h
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereSource]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereSource2]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereSource3]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereSource4]
    ../VTK
    VTK
    $ENV{HOME}/VTK
  )

#
# If we found a source tree then set use_installed_vtk to 0
#
IF (VTK_SOURCE_DIR)
  SET (USE_INSTALLED_VTK 0 CACHE BOOL "Is an installed (versus source) version of VTK used")
  #
  # Look for a binary tree
  # 
  FIND_PATH(VTK_BINARY_DIR vtkConfigure.h
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild2]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild3]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild4]
    ../VTKBIN
    ../vtkbin
    VTKBIN
    vtkbin
    $ENV{HOME}/VTKBIN
    $ENV{HOME}/vtkbin
    )
ELSE (VTK_SOURCE_DIR)
  # look for the vtk header files in installed places
  FIND_PATH(VTK_INSTALL_PATH include/vtk/vtkObject.h
    /usr/local
    /usr
    )
  FIND_PATH(VTK_INSTALL_PATH include/vtkObject.h
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Kitware\\VTK\\Nightly]
    )
  IF (VTK_INSTALL_PATH)
    SET (USE_INSTALLED_VTK 1 CACHE BOOL "Is an installed (versus source) version of VTK used")
  ENDIF (VTK_INSTALL_PATH)
ENDIF (VTK_SOURCE_DIR)

IF (USE_INSTALLED_VTK)
  # look for the vtk header files in installed places
  FIND_PATH(VTK_INSTALL_PATH include/vtk/vtkObject.h
    /usr/local
    /usr
    )
  FIND_PATH(VTK_INSTALL_PATH include/vtkObject.h
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Kitware\\VTK\\Nightly]
    )
  IF (VTK_INSTALL_PATH)
    SET (USE_INSTALLED_VTK 1 CACHE BOOL "Is an installed (versus source) version of VTK used")
  ENDIF (VTK_INSTALL_PATH)
ENDIF (USE_INSTALLED_VTK)





