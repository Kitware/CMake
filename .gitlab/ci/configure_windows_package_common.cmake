set(CMake_DOC_ARTIFACT_PREFIX "$ENV{CI_PROJECT_DIR}/build/install-doc" CACHE PATH "")

# Set up install destinations as expected by the packaging scripts.
set(CMAKE_DOC_DIR "doc/cmake" CACHE STRING "")

# Link C/C++ runtime library statically.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")

# Enable cmake-gui with static qt plugins
set(BUILD_QtDialog "TRUE" CACHE BOOL "")
set(CMake_GUI_DISTRIBUTE_WITH_Qt_LGPL "3" CACHE STRING "")
set(qt "$ENV{CI_PROJECT_DIR}/.gitlab/qt")
set(CMake_QT_STATIC_QWindowsIntegrationPlugin_LIBRARIES
  ${qt}/plugins/platforms/qwindows.lib
  ${qt}/plugins/styles/qwindowsvistastyle.lib
  ${qt}/lib/Qt5EventDispatcherSupport.lib
  ${qt}/lib/Qt5FontDatabaseSupport.lib
  ${qt}/lib/Qt5ThemeSupport.lib
  ${qt}/lib/qtfreetype.lib
  ${qt}/lib/qtlibpng.lib
  imm32.lib
  wtsapi32.lib
  CACHE STRING "")
set(CMAKE_PREFIX_PATH "${qt}" CACHE STRING "")

# Disable ccmake.
set(BUILD_CursesDialog "OFF" CACHE BOOL "")

set(CMAKE_SKIP_BOOTSTRAP_TEST "TRUE" CACHE STRING "")
set(CMake_TEST_Java OFF CACHE BOOL "")
set(CMake_TEST_Qt5 OFF CACHE BOOL "")
set(CMake_TEST_Qt6 OFF CACHE BOOL "")
set(Python_FIND_REGISTRY NEVER CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_common.cmake")
