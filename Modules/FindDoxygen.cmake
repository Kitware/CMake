# - This module looks for Doxygen and the path to Graphviz's dot
# Doxygen is a documentation generation tool see http://www.doxygen.org
# With the OS X GUI version, it likes to be installed to /Applications and
# it contains the doxygen executable in the bundle. In the versions I've 
# seen, it is located in Resources, but in general, more often binaries are 
# located in MacOS. This code sets the following variables:
#  DOXYGEN_EXECUTABLE     = The path to the doxygen command.
#  DOXYGEN_DOT_EXECUTABLE = The path to the dot program used by doxygen.
#  DOXYGEN_DOT_PATH       = The path to dot not including the executable
#  DOXYGEN = same as DOXYGEN_EXECUTABLE for backwards compatibility
#  DOT = same as DOXYGEN_DOT_EXECUTABLE for backwards compatibility

# The official Doxygen.app that is distributed for OS X uses non-standard 
# conventions. Instead of the command-line "doxygen" tool being placed in
# Doxygen.app/Contents/MacOS, "Doxywizard" is placed there instead and 
# "doxygen" is actually placed in Contents/Resources. This is most likely
# to accomodate people who double-click on the Doxygen.app package and expect
# to see something happen. However, the CMake backend gets horribly confused
# by this. Once CMake sees the bundle, it indiscrimately uses Doxywizard
# as the executable to use. The only work-around I have found is to disable
# the app-bundle feature for only this command.
# Save the old setting
SET(TEMP_DOXYGEN_SAVE_CMAKE_FIND_APPBUNDLE ${CMAKE_FIND_APPBUNDLE})
# Disable the App-bundle detection feature
SET(CMAKE_FIND_APPBUNDLE "NEVER")
# For backwards compatibility support
# DOXYGEN_FIND_QUIETLY, but it should have been
# Doxygen_FIND_QUIETLY.  
IF(Doxygen_FIND_QUIETLY)
  SET(DOXYGEN_FIND_QUIETLY TRUE)
ENDIF(Doxygen_FIND_QUIETLY)

IF (NOT DOXYGEN_FIND_QUIETLY)
  MESSAGE(STATUS "Looking for doxygen...")
ENDIF (NOT DOXYGEN_FIND_QUIETLY)

FIND_PROGRAM(DOXYGEN_EXECUTABLE
  NAMES doxygen
  PATHS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\doxygen_is1;Inno Setup: App Path]/bin"
  /Applications/Doxygen.app/Contents/Resources
  /Applications/Doxygen.app/Contents/MacOS
  DOC "Doxygen documentation generation tool (http://www.doxygen.org)"
)

IF (DOXYGEN_EXECUTABLE)
  SET (DOXYGEN_FOUND "YES")
  IF (NOT DOXYGEN_FIND_QUIETLY)
    MESSAGE(STATUS "Looking for doxygen... - found ${DOXYGEN_EXECUTABLE}")
  ENDIF (NOT DOXYGEN_FIND_QUIETLY)
ELSE (DOXYGEN_EXECUTABLE)
  IF (NOT DOXYGEN_FIND_QUIETLY)
    IF (DOXYGEN_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Looking for doxygen... - NOT found")
    ELSE (DOXYGEN_FIND_REQUIRED)
      MESSAGE(STATUS "Looking for doxygen... - NOT found")
    ENDIF (DOXYGEN_FIND_REQUIRED)
  ENDIF (NOT DOXYGEN_FIND_QUIETLY)
ENDIF (DOXYGEN_EXECUTABLE)

# In the older versions of OS X Doxygen, dot was included with the 
# Doxygen bundle. But the new versions place make you download Graphviz.app
# which contains dot in its bundle.
IF (NOT DOXYGEN_FIND_QUIETLY)
  MESSAGE(STATUS "Looking for dot tool...")
ENDIF (NOT DOXYGEN_FIND_QUIETLY)

FIND_PROGRAM(DOXYGEN_DOT_EXECUTABLE
  NAMES dot
  PATHS "$ENV{ProgramFiles}/ATT/Graphviz/bin"
  "C:/Program Files/ATT/Graphviz/bin"
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\ATT\\Graphviz;InstallPath]/bin
  /Applications/Graphviz.app/Contents/MacOS
  /Applications/Doxygen.app/Contents/Resources
  /Applications/Doxygen.app/Contents/MacOS
  DOC "Graphviz Dot tool for using Doxygen"
)

IF (NOT DOXYGEN_FIND_QUIETLY)
  IF (DOXYGEN_DOT_EXECUTABLE)
    MESSAGE(STATUS "Looking for dot tool... - found ${DOXYGEN_DOT_EXECUTABLE}")
    # The Doxyfile wants the path to Dot, not the entire path and executable
    GET_FILENAME_COMPONENT(DOXYGEN_DOT_PATH "${DOXYGEN_DOT_EXECUTABLE}" PATH CACHE)
  ELSE (DOXYGEN_DOT_EXECUTABLE)
    MESSAGE(STATUS "Looking for dot tool... - NOT found")
  ENDIF (DOXYGEN_DOT_EXECUTABLE)
ENDIF (NOT DOXYGEN_FIND_QUIETLY)


# Restore the old app-bundle setting setting
SET(CMAKE_FIND_APPBUNDLE ${TEMP_DOXYGEN_SAVE_CMAKE_FIND_APPBUNDLE})

# Backwards compatibility for CMake4.3 and less
SET (DOXYGEN ${DOXYGEN_EXECUTABLE} )
SET (DOT ${DOXYGEN_DOT_EXECUTABLE} )

MARK_AS_ADVANCED(
  DOXYGEN_FOUND
  DOXYGEN_EXECUTABLE
  DOXYGEN_DOT_FOUND
  DOXYGEN_DOT_EXECUTABLE
  DOXYGEN_DOT_PATH
  )
