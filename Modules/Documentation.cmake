# DocumentationVTK.cmake
#
# This file provides support for the VTK documentation framework.
# It relies on several tools (Doxygen, Perl, etc).

#
# Build the documentation ?
#
OPTION(BUILD_DOCUMENTATION "Build the documentation (Doxygen)." OFF)
MARK_AS_ADVANCED(BUILD_DOCUMENTATION)

IF (BUILD_DOCUMENTATION)

  #
  # Check for the tools
  #
  INCLUDE(FindUnixCommands)
  INCLUDE(FindDoxygen)
  INCLUDE(FindGnuplot)
  INCLUDE(FindHTMLHelp)
  INCLUDE(FindPerl)
  INCLUDE(FindWget)

  OPTION(DOCUMENTATION_HTML_HELP 
    "Build the HTML Help file (CHM)." OFF)

  OPTION(DOCUMENTATION_HTML_TARZ 
    "Build a compressed tar archive of the HTML doc." OFF)

  MARK_AS_ADVANCED(
    DOCUMENTATION_HTML_HELP
    DOCUMENTATION_HTML_TARZ
    )

  # 
  # The documentation process is controled by a batch file.
  # We will probably need bash to create the custom target
  #

ENDIF (BUILD_DOCUMENTATION)
