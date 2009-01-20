# - This module looks for Doxygen and the path to Graphviz's dot
# Doxygen is a documentation generation tool see http://www.doxygen.org
#
# This module accepts the following optional variables:
#
#   DOXYGEN_SKIP_DOT       = If true this module will skip trying to find Dot
#
# This modules defines the following variables:
#
#   DOXYGEN_EXECUTABLE     = The path to the doxygen command.
#   DOXYGEN_FOUND          = Was Doxygen found or not?
#
#   DOXYGEN_DOT_EXECUTABLE = The path to the dot program used by doxygen.
#   DOXYGEN_DOT_FOUND      = Was Dot found or not?
#   DOXYGEN_DOT_PATH       = The path to dot not including the executable
#
# Details for OSX Users:
#     With the OS X GUI version, it likes to be installed to /Applications and
#     it contains the doxygen executable in the bundle. In the versions I've 
#     seen, it is located in Resources, but in general, more often binaries are 
#     located in MacOS.
#
#     The official Doxygen.app that is distributed for OS X uses non-standard 
#     conventions. Instead of the command-line "doxygen" tool being placed in
#     Doxygen.app/Contents/MacOS, "Doxywizard" is placed there instead and 
#     "doxygen" is actually placed in Contents/Resources. This is most likely
#     to accomodate people who double-click on the Doxygen.app package and expect
#     to see something happen. However, the CMake backend gets horribly confused
#     by this. Once CMake sees the bundle, it indiscrimately uses Doxywizard
#     as the executable to use. The only work-around I have found is to disable
#     the app-bundle feature for only this command.
if(APPLE)
    #  Save the old setting
    SET(TEMP_DOXYGEN_SAVE_CMAKE_FIND_APPBUNDLE ${CMAKE_FIND_APPBUNDLE})
    # Disable the App-bundle detection feature
    SET(CMAKE_FIND_APPBUNDLE "NEVER")
endif()
#     FYI:
#     In the older versions of OS X Doxygen, dot was included with the 
#     Doxygen bundle. But the new versions place make you download Graphviz.app
#     which contains dot in its bundle.
# ============== End OSX stuff ================


# For backwards compatibility support
# DOXYGEN_FIND_QUIETLY, but it should have been
# Doxygen_FIND_QUIETLY.  
IF(Doxygen_FIND_QUIETLY)
  SET(DOXYGEN_FIND_QUIETLY TRUE)
ENDIF(Doxygen_FIND_QUIETLY)

#
# Find Doxygen...
#

FIND_PROGRAM(DOXYGEN_EXECUTABLE
  NAMES doxygen
  PATHS
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\doxygen_is1;Inno Setup: App Path]/bin"
    /Applications/Doxygen.app/Contents/Resources
    /Applications/Doxygen.app/Contents/MacOS
  DOC "Doxygen documentation generation tool (http://www.doxygen.org)"
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DOXYGEN DEFAULT_MSG DOXYGEN_EXECUTABLE)

#
# Find Dot...
#

IF(NOT DOXYGEN_SKIP_DOT)
  FIND_PROGRAM(DOXYGEN_DOT_EXECUTABLE
    NAMES dot
    PATHS 
      "$ENV{ProgramFiles}/Graphviz 2.21/bin"
      "C:/Program Files/Graphviz 2.21/bin"
      "$ENV{ProgramFiles}/ATT/Graphviz/bin"
      "C:/Program Files/ATT/Graphviz/bin"
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\ATT\\Graphviz;InstallPath]/bin
      /Applications/Graphviz.app/Contents/MacOS
      /Applications/Doxygen.app/Contents/Resources
      /Applications/Doxygen.app/Contents/MacOS
    DOC "Graphviz Dot tool for using Doxygen"
  )
  
  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(DOXYGEN_DOT DEFAULT_MSG DOXYGEN_DOT_EXECUTABLE)
  
  if(DOXYGEN_DOT_EXECUTABLE)
    # The Doxyfile wants the path to Dot, not the entire path and executable
    get_filename_component(DOXYGEN_DOT_PATH "${DOXYGEN_DOT_EXECUTABLE}" PATH CACHE)
  endif()
  
endif(NOT DOXYGEN_SKIP_DOT)

#
# Backwards compatibility...
#

if(APPLE)
  # Restore the old app-bundle setting setting
  SET(CMAKE_FIND_APPBUNDLE ${TEMP_DOXYGEN_SAVE_CMAKE_FIND_APPBUNDLE})
endif()

# Maintain the _FOUND variables as "YES" or "NO" for backwards compatibility
# (allows people to stuff them directly into Doxyfile with configure_file())
if(DOXYGEN_FOUND)
  set(DOXYGEN_FOUND "YES")
else()
  set(DOXYGEN_FOUND "NO")
endif()
if(DOXYGEN_DOT_FOUND)
  set(DOXYGEN_DOT_FOUND "YES")
else()
  set(DOXYGEN_DOT_FOUND "NO")
endif()

# Backwards compatibility for CMake4.3 and less
SET (DOXYGEN ${DOXYGEN_EXECUTABLE} )
SET (DOT ${DOXYGEN_DOT_EXECUTABLE} )

MARK_AS_ADVANCED(
  DOXYGEN_EXECUTABLE
  DOXYGEN_DOT_EXECUTABLE
  DOXYGEN_DOT_PATH
  )
