cmake_minimum_required(VERSION 4.0)

project(CPackAppImageGenerator
  LANGUAGES CXX
  VERSION "1.2.3"
)

add_executable(app main.cpp)

install(TARGETS app
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

install(FILES com.example.app.desktop DESTINATION share/applications)
install(FILES ApplicationIcon.png DESTINATION share/icons/hicolor/64x64/apps)

# Create AppImage package
set(CPACK_GENERATOR AppImage)
set(CPACK_PACKAGE_NAME GeneratorTest)
set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "ACME Inc")
set(CPACK_PACKAGE_DESCRIPTION "An AppImage package for testing CMake's CPack AppImage generator")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A test AppImage package")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.example.com")

# AppImage generator variables
set(CPACK_PACKAGE_ICON ApplicationIcon.png)

include(CPack)
