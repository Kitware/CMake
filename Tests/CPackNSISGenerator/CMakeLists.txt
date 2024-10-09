cmake_minimum_required(VERSION 3.16)

project(CPackNSISGenerator)

add_executable(hello main.cpp)

install(TARGETS hello
  ARCHIVE DESTINATION .
  RUNTIME DESTINATION .
  LIBRARY DESTINATION .
  BUNDLE DESTINATION .)

# Component that is a reserved name on Windows.
# See https://learn.microsoft.com/en-us/windows/win32/fileio/naming-a-file
install(
  DIRECTORY .
  DESTINATION txt
  COMPONENT CON
  FILES_MATCHING PATTERN *.txt)
# Component name that is similar to a reserved name on Windows.
install(
  DIRECTORY .
  DESTINATION txt
  COMPONENT Console
  FILES_MATCHING PATTERN *.txt)
# Component name that is strongly discouraged on Windows.
install(
  DIRECTORY .
  DESTINATION txt
  COMPONENT EndsWithDot.
  FILES_MATCHING PATTERN *.txt)

set(CPACK_NSIS_MUI_HEADERIMAGE "${PROJECT_SOURCE_DIR}\\\\header-image.bmp")
set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}\\\\header-icon.bmp")
set(CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}\\\\install.ico")
set(CPACK_NSIS_MUI_UNIICON "${PROJECT_SOURCE_DIR}\\\\uninstall.ico")
set(CPACK_GENERATOR "NSIS")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)
set(CPACK_NSIS_BRANDING_TEXT "CMake branding text")
set(CPACK_NSIS_BRANDING_TEXT_TRIM_POSITION "RIGHT")
set(CPACK_NSIS_IGNORE_LICENSE_PAGE ON)

include(CPack)
