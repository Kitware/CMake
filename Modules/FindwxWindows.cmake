#
# This module finds if wxWindows is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  WXWINDOWS_STATIC_LIBRARY  = full path to the wxWindows static library
#  WXWINDOWS_SHARED_LIBRARY  = full path to the wxWindows shared import library
#  WXWINDOWS_INCLUDE_PATH    = path to wx.h

IF(WIN32)

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

  IF(WXWINDOWS_SHARED_LIBRARY)
    OPTION(WXWINDOWS_USE_SHARED_LIBS 
           "Use shared versions of wxWindows libraries" ON)
    MARK_AS_ADVANCED(WXWINDOWS_USE_SHARED_LIBS)
  ENDIF(WXWINDOWS_SHARED_LIBRARY)

  IF(WXWINDOWS_USE_SHARED_LIBS)
    SET(WXWINDOWS_LIBRARY ${WXWINDOWS_SHARED_LIBRARY})
  ELSE(WXWINDOWS_USE_SHARED_LIBS)
    SET(WXWINDOWS_LIBRARY ${WXWINDOWS_STATIC_LIBRARY})
  ENDIF(WXWINDOWS_USE_SHARED_LIBS)

ELSE(WIN32)

  FIND_PROGRAM(CMAKE_WX_CONFIG wx-config)
  # This has to be done at some point
  #EXEC_PROGRAM(wx-config ARGS --cflags OUTPUT_VARIABLE WXWINDOWS_COMPILE_FLAGS RETURN_VALUE WX_CFLAGS_)
  #EXEC_PROGRAM(wx-config ARGS --libs OUTPUT_VARIABLE WXWINDOWS_LINK_FLAGS RETURN_VALUE WX_LIBS_RVAL)
  #IF(WX_CFLAGS_RVAL)
    SET(CMAKE_WX_CXX_FLAGS "`${CMAKE_WX_CONFIG} --cflags`")
  #ELSE(WX_CFLAGS_RVAL)
  #  MESSAGE("Cannot find \"wx-config\" or wxWindows is not configured properly")
  #ENDIF(WX_CFLAGS_RVAL)
  SET(CMAKE_WX_LIBRARIES "`${CMAKE_WX_CONFIG} --libs`")

ENDIF(WIN32)  

MARK_AS_ADVANCED(
  WXWINDOWS_STATIC_LIBRARY
  WXWINDOWS_SHARED_LIBRARY
  WXWINDOWS_INCLUDE_PATH
)
