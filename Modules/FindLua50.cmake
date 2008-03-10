# Locate Lua library
# This module defines
#  LUA_LIBRARIES, both lua and lualib
#  LUA_FOUND, if false, do not try to link to Lua 
#  LUA_INCLUDE_DIR, where to find lua.h and lualib.h (and probably lauxlib.h)
#
# Note that the expected include convention is
#  #include "lua.h"
# and not
#  #include <lua/lua.h>
# This is because, the lua location is not standardized and may exist
# in locations other than lua/


FIND_PATH(LUA_INCLUDE_DIR lua.h
  PATHS
  $ENV{LUA_DIR}
  NO_DEFAULT_PATH
  PATH_SUFFIXES include/lua50 include/lua5.0 include/lua5 include/lua include
)

FIND_PATH(LUA_INCLUDE_DIR lua.h
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  PATH_SUFFIXES include/lua50 include/lua5.0 include/lua5 include/lua include
)

FIND_LIBRARY(LUA_LIBRARY_lua 
  NAMES lua50 lua5.0 lua5 lua
  PATHS
  $ENV{LUA_DIR}
  NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

FIND_LIBRARY(LUA_LIBRARY_lua
  NAMES lua50 lua5.0 lua5 lua
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
    PATH_SUFFIXES lib64 lib
)

# In an OS X framework, lualib is usually included as part of the framework
# (like GLU in OpenGL.framework)
IF(${LUA_LIBRARY_lua} MATCHES "framework")
  SET( LUA_LIBRARIES "${LUA_LIBRARY_lua}" CACHE STRING "Lua framework")
ELSE(${LUA_LIBRARY_lua} MATCHES "framework")
  FIND_LIBRARY(LUA_LIBRARY_lualib 
    NAMES lualib50 lualib5.0 lualib5 lualib
    PATHS
    $ENV{LUALIB_DIR}
    $ENV{LUA_DIR}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
  )

  FIND_LIBRARY(LUA_LIBRARY_lualib 
    NAMES lualib50 lualib5.0 lualib5 lualib
    PATHS
    /usr/local
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt
    PATH_SUFFIXES lib64 lib
  )
  IF(LUA_LIBRARY_lualib AND LUA_LIBRARY_lua)
    # include the math library for Unix
    IF(UNIX AND NOT APPLE)
      FIND_LIBRARY(MATH_LIBRARY_FOR_LUA m)
      SET( LUA_LIBRARIES "${LUA_LIBRARY_lualib};${LUA_LIBRARY_lua};${MATH_LIBRARY_FOR_LUA}" CACHE STRING "This is the concatentation of lua and lualib libraries")
    # For Windows and Mac, don't need to explicitly include the math library
    ELSE(UNIX AND NOT APPLE)
      SET( LUA_LIBRARIES "${LUA_LIBRARY_lualib};${LUA_LIBRARY_lua}" CACHE STRING "This is the concatentation of lua and lualib libraries")
    ENDIF(UNIX AND NOT APPLE)
  ENDIF(LUA_LIBRARY_lualib AND LUA_LIBRARY_lua)
ENDIF(${LUA_LIBRARY_lua} MATCHES "framework")


INCLUDE(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LUA_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Lua50  DEFAULT_MSG  LUA_LIBRARIES LUA_INCLUDE_DIR)

MARK_AS_ADVANCED(LUA_INCLUDE_DIR LUA_LIBRARIES)

