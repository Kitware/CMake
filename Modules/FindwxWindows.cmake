#
# This module finds if wxWindows is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  WXWINDOWS_STATIC_LIBRARY  = full path to the wxWindows static library
#  WXWINDOWS_SHARED_LIBRARY  = full path to the wxWindows shared import library
#  WXWINDOWS_INCLUDE_PATH    = path to wx.h

IF(WINDOWS)
  SET (WXWINDOWS_POSSIBLE_LIB_PATHS
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\wxWindows_is1;Inno Setup: App Path]/lib"
    $ENV{WXWIN}/lib
  )

  FIND_LIBRARY(WXWINDOWS_STATIC_LIBRARY
    NAMES wx
    PATHS ${WXWINDOWS_POSSIBLE_LIB_PATHS} 
  )

  FIND_LIBRARY(WXWINDOWS_SHARED_LIBRARY
    NAMES wx23_2 wx22_9
    PATHS ${WXWINDOWS_POSSIBLE_LIB_PATHS} 
  )

  SET (WXWINDOWS_POSSIBLE_INCLUDE_PATHS
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\wxWindows_is1;Inno Setup: App Path]/include/wx"
    $ENV{WXWIN}/include/wx
  )

  FIND_PATH(WXWINDOWS_INCLUDE_PATH
    wx.h
    ${WXWINDOWS_POSSIBLE_INCLUDE_PATHS} 
  )
ELSE(WINDOWS)
  FIND_PROGRAM(CMAKE_WX_CONFIG wx-config)
  EXEC_PROGRAM(wx-config ARGS --cflags OUTPUT_VARIABLE WXWINDOWS_COMPILE_FLAGS)
  EXEC_PROGRAM(wx-config ARGS --libs OUTPUT_VARIABLE WXWINDOWS_LINK_FLAGS)
  SET(CMAKE_WX_CXX_FLAGS "`${CMAKE_WX_CONFIG} --cflags`")
  SET(CMAKE_WX_LIBRARIES "`${CMAKE_WX_CONFIG} --libs`")
ENDIF(WINDOWS)  

MARK_AS_ADVANCED(
  WXWINDOWS_STATIC_LIBRARY
  WXWINDOWS_SHARED_LIBRARY
  WXWINDOWS_INCLUDE_PATH
  WXWINDOWS_SETUP_INCLUDE_PATH
)
