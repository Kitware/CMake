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
