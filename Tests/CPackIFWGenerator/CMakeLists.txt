cmake_minimum_required(VERSION 3.16)

project(CPackIFWGenerator)

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

set(CPACK_IFW_PRODUCT_URL "https://cmake.org/")
if(WIN32)
  set(CPACK_IFW_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/install.ico")
else()
  set(CPACK_IFW_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/BundleIcon.icns")
endif()

set(CPACK_IFW_PACKAGE_WINDOW_ICON "${PROJECT_SOURCE_DIR}/install.ico")
set(CPACK_GENERATOR "IFW")

set(CPACK_IFW_PACKAGE_PRODUCT_IMAGES
  "${PROJECT_SOURCE_DIR}/ApplicationIcon.png"
  "${PROJECT_SOURCE_DIR}/SplashScreen.png"
)

set(CPACK_IFW_PACKAGE_PRODUCT_IMAGE_URLS
  "https://www.ApplicationIcon.org"
  "https://www.SplashScreen.org"
)

include(CPack)
include(CPackIFW)
