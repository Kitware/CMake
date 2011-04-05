# CPack Example: User-selectable Installation Components
#
# In this example, we have a simple library (mylib) with an example
# application (mylibapp). We create a binary installer (a CPack Generator)
# which supports CPack components.
#
# Depending on the CPack generator and on some CPACK_xxx var values
# the generator may produce a single (NSIS, PackageMaker)
# or several package files (Archive Generators, RPM, DEB)
cmake_minimum_required(VERSION 2.8.3.20101130 FATAL_ERROR)
project(CPackComponentsForAll)

# Use GNUInstallDirs in order to enforce lib64 if needed
include(GNUInstallDirs)

# Create the mylib library
add_library(mylib mylib.cpp)

# Create the mylibapp application
add_executable(mylibapp mylibapp.cpp)
target_link_libraries(mylibapp mylib)

# Duplicate of mylibapp application
# which won't be put in any component (?mistake?)
add_executable(mylibapp2 mylibapp.cpp)
target_link_libraries(mylibapp2 mylib)

# Create installation targets. Note that we put each kind of file
# into a different component via COMPONENT. These components will
# be used to create the installation components.
install(TARGETS mylib
  ARCHIVE
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  COMPONENT libraries)
install(TARGETS mylibapp
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

# This application does not belong to any component
# thus (as of cmake 2.8.2) it will be left "uninstalled"
# by a component-aware installer unless a
# CPACK_MONOLITHIC_INSTALL=1 is set (at cmake time).
install(TARGETS mylibapp2
  RUNTIME
  DESTINATION bin)

install(FILES mylib.h
  DESTINATION include
  COMPONENT headers)

# CPack boilerplate for this project
set(CPACK_PACKAGE_NAME "MyLib")
set(CPACK_PACKAGE_CONTACT "None")
set(CPACK_PACKAGE_VENDOR "CMake.org")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MyLib - CPack Component Installation Example")
set(CPACK_PACKAGE_VERSION "1.0.2")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "2")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "CPack Component Example")

# Tell CPack all of the components to install. The "ALL"
# refers to the fact that this is the set of components that
# will be included when CPack is instructed to put everything
# into the binary installer (the default behavior).
set(CPACK_COMPONENTS_ALL applications libraries headers Unspecified)

# Set the displayed names for each of the components to install.
# These will be displayed in the list of components inside the installer.
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "MyLib Application")
set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries")
set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers")

# Provide descriptions for each of the components to install.
# When the user hovers the mouse over the name of a component,
# the description will be shown in the "Description" box in the
# installer. If no descriptions are provided, the "Description"
# box will be removed.
set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION
  "An extremely useful application that makes use of MyLib")
set(CPACK_COMPONENT_LIBRARIES_DESCRIPTION
  "Static libraries used to build programs with MyLib")
set(CPACK_COMPONENT_HEADERS_DESCRIPTION
  "C/C++ header files for use with MyLib")

# Put the components into two different groups: "Runtime" and "Development"
set(CPACK_COMPONENT_APPLICATIONS_GROUP "Runtime")
set(CPACK_COMPONENT_LIBRARIES_GROUP "Development")
set(CPACK_COMPONENT_HEADERS_GROUP "Development")

# Expand the "Development" group by default, since we have so few components.
# Also, provide this group with a description.
set(CPACK_COMPONENT_GROUP_DEVELOPMENT_EXPANDED ON)
set(CPACK_COMPONENT_GROUP_DEVELOPMENT_DESCRIPTION
  "All of the tools you'll ever need to develop software")

# It doesn't make sense to install the headers without the libraries
# (because you could never use the headers!), so make the headers component
# depend on the libraries component.
set(CPACK_COMPONENT_HEADERS_DEPENDS libraries)

# Create two installation types with pre-selected components.
# The "Developer" installation has just the library and headers,
# while the "Full" installation has everything.
set(CPACK_ALL_INSTALL_TYPES Full Developer)
set(CPACK_INSTALL_TYPE_FULL_DISPLAY_NAME "Everything")
set(CPACK_COMPONENT_LIBRARIES_INSTALL_TYPES Developer Full)
set(CPACK_COMPONENT_HEADERS_INSTALL_TYPES Developer Full)
set(CPACK_COMPONENT_APPLICATIONS_INSTALL_TYPES Full)

# We may use the CPack specific config file in order
# to tailor CPack behavior on a CPack generator specific way
# (Behavior would be different for RPM or TGZ or DEB ...)
if (NOT ("${CPackComponentWay}" STREQUAL "default"))
  # Setup project specific CPack-time CPack Config file.
  configure_file(${CPackComponentsForAll_SOURCE_DIR}/MyLibCPackConfig-${CPackComponentWay}.cmake.in
    ${CPackComponentsForAll_BINARY_DIR}/MyLibCPackConfig-${CPackComponentWay}.cmake
    @ONLY)
  set(CPACK_PROJECT_CONFIG_FILE ${CPackComponentsForAll_BINARY_DIR}/MyLibCPackConfig-${CPackComponentWay}.cmake)
endif (NOT ("${CPackComponentWay}" STREQUAL "default"))
# Include CPack to introduce the appropriate targets
include(CPack)