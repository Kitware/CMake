cmake_minimum_required(VERSION 2.8)

project(CPackWiXGenerator)

add_library(mylib mylib.cpp)

add_executable(my-libapp mylibapp.cpp)
target_link_libraries(my-libapp mylib)

add_executable(my-other-app myotherapp.cpp)

install(TARGETS mylib
  ARCHIVE
  DESTINATION lib
  COMPONENT libraries)

install(TARGETS my-libapp my-other-app
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

install(FILES mylib.h "file with spaces.h"
  DESTINATION include
  COMPONENT headers)

set(CPACK_GENERATOR "WIX")

set(CPACK_PACKAGE_NAME "MyLib")
set(CPACK_PACKAGE_VENDOR "CMake.org")
set(CPACK_PACKAGE_CONTACT "somebody@cmake.org")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "MyLib - CPack Component Installation Example")

set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "CPack Component Example")

set(CPACK_WIX_UPGRADE_GUID "BF20CE5E-7F7C-401D-8F7C-AB45E8D170E6")
set(CPACK_WIX_UNINSTALL "1")

set(CPACK_PACKAGE_EXECUTABLES
  "my-libapp" "CPack WiX Test"
  "my-other-app" "Second CPack WiX Test"
)

include(CPack)

cpack_add_install_type(Full DISPLAY_NAME "Everything")
cpack_add_install_type(Developer)

cpack_add_component_group(Runtime)

cpack_add_component_group(Development
  EXPANDED
  DESCRIPTION "All of the tools you'll ever need to develop software")

cpack_add_component(applications
  DISPLAY_NAME "MyLib Application"
  DESCRIPTION "An extremely useful application that makes use of MyLib"
  GROUP Runtime
  INSTALL_TYPES Full)

cpack_add_component(documentation
  DISPLAY_NAME "MyLib Documentation"
  DESCRIPTION "The extensive suite of MyLib Application documentation files"
  GROUP Runtime
  INSTALL_TYPES Full)

cpack_add_component(libraries
  DISPLAY_NAME "Libraries"
  DESCRIPTION "Static libraries used to build programs with MyLib"
  GROUP Development
  INSTALL_TYPES Developer Full)

cpack_add_component(headers
  DISPLAY_NAME "C++ Headers"
  DESCRIPTION "C/C++ header files for use with MyLib"
  GROUP Development
  DEPENDS libraries
  INSTALL_TYPES Developer Full)
