#
# This module finds if TCL is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  TCL_LIBRARY            = full path to the Tcl library (tcl tcl80 etc)
#  TCL_LIBRARY_DEBUG      = full path to the Tcl library (debug)
#  TCL_STUB_LIBRARY       = full path to the Tcl stub library
#  TCL_STUB_LIBRARY_DEBUG = full path to the Tcl stub library (debug)
#  TCL_INCLUDE_PATH       = path to where tcl.h can be found
#  TCL_TCLSH              = full path to the tclsh binary (tcl tcl80 etc)
#  TK_LIBRARY             = full path to the Tk library (tk tk80 etc)
#  TK_LIBRARY_DEBUG       = full path to the Tk library (debug)
#  TK_STUB_LIBRARY        = full path to the Tk stub library
#  TK_STUB_LIBRARY_DEBUG  = full path to the Tk stub library (debug)
#  TK_INCLUDE_PATH        = path to where tk.h can be found
#  TK_INTERNAL_PATH       = path to where tkWinInt.h can be found
#  TK_WISH                = full path to the wish binary (wish wish80 etc)
#

INCLUDE(${CMAKE_ROOT}/Modules/FindTclsh.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/FindWish.cmake)

GET_FILENAME_COMPONENT(TCL_TCLSH_PATH ${TCL_TCLSH} PATH)

GET_FILENAME_COMPONENT(TK_WISH_PATH ${TK_WISH} PATH)

SET (TCLTK_POSSIBLE_LIB_PATHS
  "${TCL_TCLSH_PATH}/../lib"
  "${TK_WISH_PATH}/../lib"
  /usr/lib 
  /usr/local/lib
  "C:/Program Files/Tcl/lib" 
  "C:/Tcl/lib" 
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.4;Root]/lib
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.3;Root]/lib
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.2;Root]/lib
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.0;Root]/lib
)

FIND_LIBRARY(TCL_LIBRARY
  NAMES tcl tcl84 tcl8.4 tcl83 tcl8.3 tcl82 tcl8.2 tcl80 tcl8.0
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
)

FIND_LIBRARY(TCL_LIBRARY_DEBUG
  NAMES tcld tcl84d tcl8.4d tcl83d tcl8.3d tcl82d tcl8.2d tcl80d tcl8.0d
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
)

FIND_LIBRARY(TCL_STUB_LIBRARY
  NAMES tclstub tclstub84 tclstub8.4 tclstub83 tclstub8.3 tclstub82 tclstub8.2 tclstub80 tclstub8.0
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
)

FIND_LIBRARY(TCL_STUB_LIBRARY_DEBUG
  NAMES tclstubd tclstub84d tclstub8.4d tclstub83d tclstub8.3d tclstub82d tclstub8.2d tclstub80d tclstub8.0d
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
)

FIND_LIBRARY(TK_LIBRARY 
  NAMES tk tk84 tk8.4 tk83 tk8.3 tk82 tk8.2 tk80 tk8.0
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
)

FIND_LIBRARY(TK_LIBRARY_DEBUG
  NAMES tkd tk84d tk8.4d tk83d tk8.3d tk82d tk8.2d tk80d tk8.0d
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
)

FIND_LIBRARY(TK_STUB_LIBRARY 
  NAMES tkstub tkstub84 tkstub8.4 tkstub83 tkstub8.3 tkstub82 tkstub8.2 tkstub80 tkstub8.0
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
)

FIND_LIBRARY(TK_STUB_LIBRARY_DEBUG
  NAMES tkstubd tkstub84d tkstub8.4d tkstub83d tkstub8.3d tkstub82d tkstub8.2d tkstub80d tkstub8.0d
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
)

SET (TCLTK_POSSIBLE_INCLUDE_PATHS
  ~/Library/Frameworks/Tcl.framework/Headers
  ~/Library/Frameworks/Tk.framework/Headers
  ~/Library/Frameworks/Tk.framework/PrivateHeaders
  /Library/Frameworks/Tcl.framework/Headers
  /Library/Frameworks/Tk.framework/Headers
  /Library/Frameworks/Tk.framework/PrivateHeaders
  "${TCL_TCLSH_PATH}/../include"
  "${TK_WISH_PATH}/../include"
  /usr/include 
  /usr/local/include
  /usr/include/tcl8.4
  /usr/include/tcl8.3
  /usr/include/tcl8.2
  /usr/include/tcl8.0
  "C:/Program Files/Tcl/include" 
  "C:/Tcl/include" 
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.4;Root]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.3;Root]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.2;Root]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.0;Root]/include
)

FIND_PATH(TCL_INCLUDE_PATH tcl.h 
  ${TCLTK_POSSIBLE_INCLUDE_PATHS}
)

FIND_PATH(TK_INCLUDE_PATH tk.h 
  ${TCLTK_POSSIBLE_INCLUDE_PATHS}
)

IF (WIN32)
  FIND_PATH(TK_INTERNAL_PATH tkWinInt.h
    ${TCLTK_POSSIBLE_INCLUDE_PATHS}
  )
  MARK_AS_ADVANCED(TK_INTERNAL_PATH)

  MARK_AS_ADVANCED(
    TCL_TCLSH_PATH
    TK_WISH_PATH
    TCL_INCLUDE_PATH
    TK_INCLUDE_PATH
    TCL_LIBRARY
    TCL_LIBRARY_DEBUG
    TK_LIBRARY  
    TK_LIBRARY_DEBUG
    )
ENDIF(WIN32)

IF(APPLE)
  IF(EXISTS ~/Library/Frameworks/Tcl.framework)
    SET(TCL_HAVE_FRAMEWORK 1)
  ENDIF(EXISTS ~/Library/Frameworks/Tcl.framework)
  IF(EXISTS /Library/Frameworks/Tcl.framework)
    SET(TCL_HAVE_FRAMEWORK 1)
  ENDIF(EXISTS /Library/Frameworks/Tcl.framework)
  IF(EXISTS ~/Library/Frameworks/Tk.framework)
    SET(TCL_TK_HAVE_FRAMEWORK 1)
  ENDIF(EXISTS ~/Library/Frameworks/Tk.framework)
  IF(EXISTS /Library/Frameworks/Tk.framework)
    SET(TCL_TK_HAVE_FRAMEWORK 1)
  ENDIF(EXISTS /Library/Frameworks/Tk.framework)
  IF("${TCL_INCLUDE_PATH}" MATCHES "Tcl\\.framework")
    SET(TCL_LIBRARY "")
  ENDIF("${TCL_INCLUDE_PATH}" MATCHES "Tcl\\.framework")
  IF(TCL_HAVE_FRAMEWORK)
    IF(NOT TCL_LIBRARY)
      SET (TCL_LIBRARY "-framework Tcl" CACHE FILEPATH "Tcl Framework" FORCE)
    ENDIF(NOT TCL_LIBRARY)
  ENDIF(TCL_HAVE_FRAMEWORK)
  IF(TCL_TK_HAVE_FRAMEWORK)
    IF(NOT TK_LIBRARY)
      SET (TK_LIBRARY "-framework Tk" CACHE FILEPATH "Tk Framework" FORCE)
    ENDIF(NOT TK_LIBRARY)
  ENDIF(TCL_TK_HAVE_FRAMEWORK)
ENDIF(APPLE)

MARK_AS_ADVANCED(
  TCL_STUB_LIBRARY
  TCL_STUB_LIBRARY_DEBUG
  TK_STUB_LIBRARY  
  TK_STUB_LIBRARY_DEBUG
  )
