#
# Find the native ITK includes and library
#
# This module defines
#
# ITK_BINARY_PATH - where is the binary tree (only defined if SOURCE_PATH is defined)
# USE_ITK_FILE - the full path and location of the UseITK.cmake file
#

#
# Look for a binary tree
# 
FIND_PATH(ITK_BINARY_PATH UseITK.cmake
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
    ../ITKBIN
    ../itkbin
    ../Insight-vc
    ../InsightBin
    ITKBIN
    itkbin
    $ENV{HOME}/ITKBIN
    $ENV{HOME}/Insight
    $ENV{HOME}/InsightBin
    $ENV{HOME}/itkbin
    )



IF (ITK_BINARY_PATH)
  SET (USE_ITK_FILE ${ITK_BINARY_PATH}/UseITK.cmake)
ENDIF (ITK_BINARY_PATH)



