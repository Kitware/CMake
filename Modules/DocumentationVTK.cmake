# DocumentationVTK.cmake
#
# This file provides support for the VTK documentation framework.
# It relies on several tools (Doxygen, Perl, etc).

#
# Build the documentation ?
#
OPTION(BUILD_DOCUMENTATION "Build the documentation (Doxygen)." OFF)

IF (BUILD_DOCUMENTATION)

  #
  # Check for the tools
  #
  INCLUDE(${CMAKE_ROOT}/Modules/FindCygwin.cmake)
  INCLUDE(${CMAKE_ROOT}/Modules/FindDoxygen.cmake)
  INCLUDE(${CMAKE_ROOT}/Modules/FindHhc.cmake)
  INCLUDE(${CMAKE_ROOT}/Modules/FindPerl.cmake)
  INCLUDE(${CMAKE_ROOT}/Modules/FindWget.cmake)

  OPTION(DOCUMENTATION_HTML_HELP 
         "Build the HTML Help file (CHM)." OFF)

  OPTION(DOCUMENTATION_HTML_TARZ 
	 "Build a compressed tar archive of the HTML doc." OFF)

  # 
  # The documentation process is controled by a batch file.
  # We will probably need bash to create the custom target
  #
  FIND_PROGRAM(BASH bash)

ENDIF (BUILD_DOCUMENTATION)
