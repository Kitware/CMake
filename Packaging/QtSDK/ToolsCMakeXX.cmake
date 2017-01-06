# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# CMake version
include("${CMAKE_CURRENT_LIST_DIR}/../../Source/CMakeVersion.cmake")
set(CM_VER_XY ${CMake_VERSION_MAJOR}${CMake_VERSION_MINOR})
set(CM_VER_X_Y ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR})
set(CM_VER_X_Y_Z ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CMake_VERSION_PATH})

# Destiantion
set(CM_INST_PREF "Tools/CMake/${CM_VER_X_Y}")
set(CMAKE_BIN_DIR "${CM_INST_PREF}/bin"
  CACHE STRING "Location under install bin")
set(CMAKE_DATA_DIR "${CM_INST_PREF}/share/cmake-${CM_VER_X_Y}"
  CACHE STRING "Location under install data")
set(CMAKE_DOC_DIR "${CM_INST_PREF}/doc/cmake-${CM_VER_X_Y}"
  CACHE STRING "Location under install docs")
set(CMAKE_MAN_DIR "${CM_INST_PREF}/man"
  CACHE STRING "Location under install man pages")
set(CMAKE_XDGDATA_DIR "${CM_INST_PREF}/share"
  CACHE STRING "Location under install XDG specific files")

# Package
set(CMake_IFW_ROOT_COMPONENT_NAME
  "qt.tools.cmake.${CM_VER_XY}"
  CACHE STRING "QtSDK CMake tools component name")
set(CMake_IFW_ROOT_COMPONENT_DISPLAY_NAME
  "CMake ${CM_VER_X_Y}"
  CACHE STRING "QtSDK CMake tools component display name")
set(CMake_IFW_ROOT_COMPONENT_DESCRIPTION
  "CMake Build Tools ${CM_VER_X_Y_Z}"
  CACHE STRING "QtSDK CMake tools component description")
set(CMake_IFW_ROOT_COMPONENT_SCRIPT
  "${CMAKE_CURRENT_BINARY_DIR}/qt.tools.cmake.${CM_VER_XY}.qs"
  CACHE STRING "QtSDK CMake tools component display name")
set(CMake_IFW_ROOT_COMPONENT_PRIORITY
  "${CM_VER_XY}"
  CACHE STRING "QtSDK CMake tools component sorting priority")
set(CMake_IFW_ROOT_COMPONENT_DEFAULT ""
  CACHE STRING "QtSDK CMake tools component default")
set(CMake_IFW_ROOT_COMPONENT_FORCED_INSTALLATION ""
  CACHE STRING "QtSDK CMake tools component forsed installation")

# CPack
set(CPACK_GENERATOR "IFW"
  CACHE STRING "Generator to build QtSDK CMake package")
set(CPACK_PACKAGE_FILE_NAME "CMake"
  CACHE STRING "Short package name")
set(CPACK_TOPLEVEL_TAG "../QtSDK"
  CACHE STRING "QtSDK packages dir")
set(CPACK_IFW_DOWNLOAD_ALL "TRUE"
  CACHE STRING "All QtSDK components is downloaded")
set(CPACK_DOWNLOAD_SITE "file:///${CMAKE_CURRENT_BINARY_DIR}/QtSDK/IFW/CMake/repository"
  CACHE STRING "Local repository for testing")

# Script
set(SDKToolBinary "@SDKToolBinary@")
set(CM_VER_XY_DIR "@CMAKE${CM_VER_XY}_DIR@")
configure_file("${CMAKE_CURRENT_LIST_DIR}/qt.tools.cmake.xx.qs.in"
  "${CMAKE_CURRENT_BINARY_DIR}/qt.tools.cmake.${CM_VER_XY}.qs"
  @ONLY)

# Unset temporary variables
unset(CM_VER_XY)
unset(CM_VER_X_Y)
unset(CM_VER_X_Y_Z)
unset(CM_INST_PREF)
unset(SDKToolBinary)
