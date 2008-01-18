# - Find Tcl includes and libraries.
# This module finds if TCL is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#  TCL_FOUND          = Tcl was found
#  TK_FOUND           = Tk was found
#  TCLTK_FOUND        = Tcl and Tk were found
#  TCL_LIBRARY        = path to Tcl library (tcl tcl80)
#  TCL_LIBRARY_DEBUG  = path to Tcl library (debug)
#  TCL_STUB_LIBRARY   = path to Tcl stub library
#  TCL_STUB_LIBRARY_DEBUG = path to debug stub library
#  TCL_INCLUDE_PATH   = path to where tcl.h can be found
#  TCL_TCLSH          = path to tclsh binary (tcl tcl80)
#  TK_LIBRARY         = path to Tk library (tk tk80 etc)
#  TK_LIBRARY_DEBUG   = path to Tk library (debug)
#  TK_STUB_LIBRARY    = path to Tk stub library
#  TK_STUB_LIBRARY_DEBUG = path to debug Tk stub library
#  TK_INCLUDE_PATH    = path to where tk.h can be found
#  TK_INTERNAL_PATH   = path to where tkWinInt.h is found
#  TK_WISH            = full path to the wish executable

INCLUDE(CMakeFindFrameworks)
INCLUDE(FindTclsh)
INCLUDE(FindWish)

GET_FILENAME_COMPONENT(TCL_TCLSH_PATH "${TCL_TCLSH}" PATH)
GET_FILENAME_COMPONENT(TCL_TCLSH_PATH_PARENT "${TCL_TCLSH_PATH}" PATH)

GET_FILENAME_COMPONENT(TK_WISH_PATH "${TK_WISH}" PATH)
GET_FILENAME_COMPONENT(TK_WISH_PATH_PARENT "${TK_WISH_PATH}" PATH)

GET_FILENAME_COMPONENT(TCL_INCLUDE_PATH_PARENT "${TCL_INCLUDE_PATH}" PATH)
GET_FILENAME_COMPONENT(TK_INCLUDE_PATH_PARENT "${TK_INCLUDE_PATH}" PATH)

GET_FILENAME_COMPONENT(TCL_LIBRARY_PATH "${TCL_LIBRARY}" PATH)
GET_FILENAME_COMPONENT(TCL_LIBRARY_PATH_PARENT "${TCL_LIBRARY_PATH}" PATH)

GET_FILENAME_COMPONENT(TK_LIBRARY_PATH "${TK_LIBRARY}" PATH)
GET_FILENAME_COMPONENT(TK_LIBRARY_PATH_PARENT "${TK_LIBRARY_PATH}" PATH)

GET_FILENAME_COMPONENT(
  ActiveTcl_CurrentVersion 
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActiveTcl;CurrentVersion]" 
  NAME)

SET (TCLTK_POSSIBLE_LIB_PATHS
  "${TCL_TCLSH_PATH_PARENT}/lib"
  "${TK_WISH_PATH_PARENT}/lib"
  "${TCL_INCLUDE_PATH_PARENT}/lib"
  "${TK_INCLUDE_PATH_PARENT}/lib"
  "$ENV{ProgramFiles}/Tcl/Lib"
  "C:/Program Files/Tcl/lib" 
  "C:/Tcl/lib" 
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActiveTcl\\${ActiveTcl_CurrentVersion}]/lib
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.5;Root]/lib
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.4;Root]/lib
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.3;Root]/lib
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.2;Root]/lib
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.0;Root]/lib
  /usr/lib 
  /usr/local/lib
)

FIND_LIBRARY(TCL_LIBRARY
  NAMES tcl 
  tcl85 tcl8.5 
  tcl84 tcl8.4 
  tcl83 tcl8.3 
  tcl82 tcl8.2 
  tcl80 tcl8.0
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
)

FIND_LIBRARY(TCL_LIBRARY_DEBUG
  NAMES tcld 
  tcl85d tcl8.5d 
  tcl85g tcl8.5g 
  tcl84d tcl8.4d 
  tcl84g tcl8.4g 
  tcl83d tcl8.3d 
  tcl82d tcl8.2d 
  tcl80d tcl8.0d
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
)

FIND_LIBRARY(TCL_STUB_LIBRARY
  NAMES tclstub 
  tclstub85 tclstub8.5 
  tclstub84 tclstub8.4 
  tclstub83 tclstub8.3 
  tclstub82 tclstub8.2 
  tclstub80 tclstub8.0
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
)

FIND_LIBRARY(TCL_STUB_LIBRARY_DEBUG
  NAMES tclstubd 
  tclstub85d tclstub8.5d 
  tclstub85g tclstub8.5g 
  tclstub84d tclstub8.4d 
  tclstub84g tclstub8.4g 
  tclstub83d tclstub8.3d 
  tclstub82d tclstub8.2d 
  tclstub80d tclstub8.0d
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
)

FIND_LIBRARY(TK_LIBRARY 
  NAMES tk 
  tk85 tk8.5 
  tk84 tk8.4 
  tk83 tk8.3 
  tk82 tk8.2 
  tk80 tk8.0
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
)

FIND_LIBRARY(TK_LIBRARY_DEBUG
  NAMES tkd 
  tk85d tk8.5d 
  tk85g tk8.5g 
  tk84d tk8.4d 
  tk84g tk8.4g 
  tk83d tk8.3d 
  tk82d tk8.2d 
  tk80d tk8.0d
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
)

FIND_LIBRARY(TK_STUB_LIBRARY 
  NAMES tkstub 
  tkstub85 tkstub8.5 
  tkstub84 tkstub8.4 
  tkstub83 tkstub8.3 
  tkstub82 tkstub8.2 
  tkstub80 tkstub8.0
  PATHS ${TCLTK_POSSIBLE_LIB_PATHS}
)

FIND_LIBRARY(TK_STUB_LIBRARY_DEBUG
  NAMES tkstubd 
  tkstub85d tkstub8.5d 
  tkstub85g tkstub8.5g 
  tkstub84d tkstub8.4d 
  tkstub84g tkstub8.4g 
  tkstub83d tkstub8.3d 
  tkstub82d tkstub8.2d 
  tkstub80d tkstub8.0d
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

SET (TCLTK_POSSIBLE_INCLUDE_PATHS
  ${TCL_TCLSH_PATH_PARENT}/include
  ${TK_WISH_PATH_PARENT}/include
  "${TCL_LIBRARY_PATH_PARENT}/include"
  "${TK_LIBRARY_PATH_PARENT}/include"
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActiveTcl\\${ActiveTcl_CurrentVersion}]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.5;Root]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.4;Root]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.3;Root]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.2;Root]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Scriptics\\Tcl\\8.0;Root]/include
  "$ENV{ProgramFiles}/Tcl/include"
  "C:/Program Files/Tcl/include"
  C:/Tcl/include
  /usr/include
  /usr/local/include
  /usr/include/tcl8.5
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
ENDIF(WIN32)

# handle the QUIETLY and REQUIRED arguments and set TIFF_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(TCL DEFAULT_MSG TCL_LIBRARY TCL_INCLUDE_PATH)
SET(TCLTK_FIND_REQUIRED ${TCL_FIND_REQUIRED})
SET(TCLTK_FIND_QUIETLY  ${TCL_FIND_QUIETLY})
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TCLTK DEFAULT_MSG TCL_LIBRARY TCL_INCLUDE_PATH TK_LIBRARY TK_INCLUDE_PATH)
SET(TK_FIND_REQUIRED ${TCL_FIND_REQUIRED})
SET(TK_FIND_QUIETLY  ${TCL_FIND_QUIETLY})
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TK DEFAULT_MSG TK_LIBRARY TK_INCLUDE_PATH)


MARK_AS_ADVANCED(
  TCL_TCLSH_PATH
  TK_WISH_PATH
  TCL_INCLUDE_PATH
  TK_INCLUDE_PATH
  TCL_LIBRARY
  TCL_LIBRARY_DEBUG
  TK_LIBRARY
  TK_LIBRARY_DEBUG
  TCL_STUB_LIBRARY
  TCL_STUB_LIBRARY_DEBUG
  TK_STUB_LIBRARY  
  TK_STUB_LIBRARY
  TK_STUB_LIBRARY_DEBUG
  )

