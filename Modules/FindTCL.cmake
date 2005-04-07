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

INCLUDE(CMakeFindFrameworks)
INCLUDE(FindTclsh)
INCLUDE(FindWish)

GET_FILENAME_COMPONENT(TCL_TCLSH_PATH "${TCL_TCLSH}" PATH)
GET_FILENAME_COMPONENT(TK_WISH_PATH "${TK_WISH}" PATH)

SET (TCLTK_POSSIBLE_LIB_PATHS
  "${TCL_TCLSH_PATH}/../lib"
  "${TK_WISH_PATH}/../lib"
  "${TCL_INCLUDE_PATH}/../lib"
  "${TK_INCLUDE_PATH}/../lib"
  "C:/Program Files/Tcl/lib" 
  "C:/Tcl/lib" 
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.4;Root]/lib
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.3;Root]/lib
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.2;Root]/lib
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.0;Root]/lib
  /usr/lib 
  /usr/local/lib
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

CMAKE_FIND_FRAMEWORKS(Tcl)
CMAKE_FIND_FRAMEWORKS(Tk)

SET(TCL_FRAMEWORK_INCLUDES)
IF(Tcl_FRAMEWORKS)
  IF(NOT TCL_INCLUDE_PATH)
    FOREACH(dir ${Tcl_FRAMEWORKS})
      SET(TCL_FRAMEWORK_INCLUDES ${TCL_FRAMEWORK_INCLUDES} ${dir}/Headers)
    ENDFOREACH(dir)
  ENDIF(NOT TCL_INCLUDE_PATH)
ENDIF(Tcl_FRAMEWORKS)

SET(TK_FRAMEWORK_INCLUDES)
IF(Tk_FRAMEWORKS)
  IF(NOT TK_INCLUDE_PATH)
    FOREACH(dir ${Tk_FRAMEWORKS})
      SET(TK_FRAMEWORK_INCLUDES ${TK_FRAMEWORK_INCLUDES}
        ${dir}/Headers ${dir}/PrivateHeaders)
    ENDFOREACH(dir)
  ENDIF(NOT TK_INCLUDE_PATH)
ENDIF(Tk_FRAMEWORKS)

GET_FILENAME_COMPONENT(TCL_LIBRARY_PATH "${TCL_LIBRARY}" PATH)
GET_FILENAME_COMPONENT(TK_LIBRARY_PATH "${TK_LIBRARY}" PATH)

SET (TCLTK_POSSIBLE_INCLUDE_PATHS
  ${TCL_TCLSH_PATH}/../include
  ${TK_WISH_PATH}/../include
  "${TCL_LIBRARY_PATH}/../include"
  "${TK_LIBRARY_PATH}/../include"
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.4;Root]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.3;Root]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.2;Root]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.0;Root]/include
  "C:/Program Files/Tcl/include"
  C:/Tcl/include
  /usr/include
  /usr/local/include
  /usr/include/tcl8.4
  /usr/include/tcl8.3
  /usr/include/tcl8.2
  /usr/include/tcl8.0
)

FIND_PATH(TCL_INCLUDE_PATH tcl.h
  ${TCL_FRAMEWORK_INCLUDES} ${TCLTK_POSSIBLE_INCLUDE_PATHS}
)

FIND_PATH(TK_INCLUDE_PATH tk.h
  ${TK_FRAMEWORK_INCLUDES} ${TCLTK_POSSIBLE_INCLUDE_PATHS}
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

IF(Tcl_FRAMEWORKS)
  # If we are using the Tcl framework, link to it.
  IF("${TCL_INCLUDE_PATH}" MATCHES "Tcl\\.framework")
    SET(TCL_LIBRARY "")
  ENDIF("${TCL_INCLUDE_PATH}" MATCHES "Tcl\\.framework")
  IF(NOT TCL_LIBRARY)
    SET (TCL_LIBRARY "-framework Tcl" CACHE FILEPATH "Tcl Framework" FORCE)
  ENDIF(NOT TCL_LIBRARY)
ENDIF(Tcl_FRAMEWORKS)

IF(Tk_FRAMEWORKS)
  # If we are using the Tk framework, link to it.
  IF("${TK_INCLUDE_PATH}" MATCHES "Tk\\.framework")
    SET(TK_LIBRARY "")
  ENDIF("${TK_INCLUDE_PATH}" MATCHES "Tk\\.framework")
  IF(NOT TK_LIBRARY)
    SET (TK_LIBRARY "-framework Tk" CACHE FILEPATH "Tk Framework" FORCE)
  ENDIF(NOT TK_LIBRARY)
ENDIF(Tk_FRAMEWORKS)

MARK_AS_ADVANCED(
  TCL_STUB_LIBRARY
  TCL_STUB_LIBRARY_DEBUG
  TK_STUB_LIBRARY  
  TK_STUB_LIBRARY_DEBUG
  )

IF(TCL_INCLUDE_PATH)
  IF(TK_INCLUDE_PATH)
    IF(TCL_LIBRARY)
      IF(TK_LIBRARY)
        SET(TCL_FOUND 1)
      ENDIF(TK_LIBRARY)
    ENDIF(TCL_LIBRARY)
  ENDIF(TK_INCLUDE_PATH)
ENDIF(TCL_INCLUDE_PATH)
