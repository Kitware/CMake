#
# This module add the VTK include paths to a project
# It should be included after the FindVTK module
#
IF (USE_INSTALLED_VTK)
  IF (VTK_INSTALL_PATH)
    INCLUDE_DIRECTORIES ( ${VTK_INSTALL_PATH}/include ${VTK_INSTALL_PATH}/include/vtk )
  ENDIF (VTK_INSTALL_PATH)
ELSE (USE_INSTALLED_VTK)
  IF (VTK_SOURCE_PATH)
    INCLUDE_DIRECTORIES(
       ${VTK_SOURCE_PATH}/Common
       ${VTK_SOURCE_PATH}/Filtering
       ${VTK_SOURCE_PATH}/IO
       ${VTK_SOURCE_PATH}/Graphics
       ${VTK_SOURCE_PATH}/Imaging
       ${VTK_SOURCE_PATH}/Rendering
       ${VTK_SOURCE_PATH}/Hybrid
       ${VTK_SOURCE_PATH}/Patented
       ${VTK_SOURCE_PATH}/Parallel)
  ENDIF (VTK_SOURCE_PATH)
  IF (VTK_BIN_PATH)
    INCLUDE_DIRECTORIES( ${VTK_BIN_PATH) )
  ENDIF (VTK_BIN_PATH)
ENDIF (USE_INSTALLED_VTK)
