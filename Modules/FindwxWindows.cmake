##
## This module finds if wxWindows is installed and determines where the
## include files and libraries are. It also determines what the name of
## the library is. This code sets the following variables:
##
## ------------------------------------------------------------------
##
## WXWINDOWS_FOUND            = system has WxWindows (in desired config. build)
##
## WXWINDOWS_LIBRARIES        = full path to the wxWindows libraries
##                              on Unix/Linux with additional linker flags from
##                              "wx-config --libs"
## 
## CMAKE_WXWINDOWS_CXX_FLAGS  = Unix compiler flags for wxWindows, essentially
##                              "`wx-config --cxxflags`"
##
## WXWINDOWS_INCLUDE_DIR      = where to find headers "wx/wx.h" "wx/setup.h"
##
## WXWINDOWS_LINK_DIRECTORIES = link directories, useful for rpath on Unix
##
## WXWINDOWS_DEFINITIONS      = extra defines
##
## deprecated: 
##   * CMAKE_WX_CAN_COMPILE
##   * WXWINDOWS_LIBRARY
##   * CMAKE_WX_CXX_FLAGS
##   * WXWINDOWS_INCLUDE_PATH
##
## ------------------------------------------------------------------
##
## USAGE: 
##   # for convenience include Use_wxWindows.cmake in your projects
##     CMakeLists.txt: 
##   
##   # if you need OpenGL support please
## SET(WXWINDOWS_USE_GL 1) 
##   # in your CMakeLists.txt *before* you include this file.
##
##   # just include Use_wxWindows.cmake 
##   # in your projects CMakeLists.txt
## INCLUDE( ${CMAKE_ROOT}/Modules/Use_wxWindows.cmake)
## 
## ------------------------------------------------------------------
## Author Jan Woetzel <http://www.mip.informatik.uni-kiel.de/~jw> (07/2003)
## ------------------------------------------------------------------
## 
## -changed variable names to conventions from cmakes readme.txt (Jan Woetzel
##  07/07/2003)
## -added definition WINVER for WIN32 (Jan Woetzel 07/07//2003)
## -added IF(CMAKE_WXWINDOWS_CONFIG_EXECUTABLE) found and changed
##  CMAKE_WX_CONFIG to CMAKE_WXWINDOWS_CONFIG_EXECUTABLE (Jan Woetzel
##  07/22/2003)
## -removed OPTION for CMAKE_WXWINDOWS_USE_GL. Force the developer to SET it.
## 
## status: 
## tested with:
##   -cmake 1.6.7, Linux (Suse 7.3), wxWindows 2.4.0, gcc 2.95
##   -cmake 1.6.7, Linux (Suse 8.2), wxWindows 2.4.0, gcc 3.3
##   -cmake 1.6.7, Linux (Suse 8.2), wxWindows 2.4.1-patch1,  gcc 3.3
##   -cmake 1.6.7, MS Windows XP home, wxWindows 2.4.1, MS Visual Studio .net 7
##    2002 (static build)
## 
## TODO: 
##  -OPTION for unicode builds 
##  -change WXWINDOWS_USE_GL to use FindOpenGL.cmake or let the user do it
##  -testing of DLL linking under MS WIN32
## 



IF(WIN32)
  
  ## ######################################################################
  ##
  ## Windows specific:
  ##
  ## candidates for root/base directory of wxwindows
  ## should have subdirs include and lib containing include/wx/wx.h
  ## fix the root dir to avoid mixing of headers/libs from different
  ## versions/builds:
  
  SET (WXWINDOWS_POSSIBLE_ROOT_PATHS
    $ENV{WXWIN}
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\wxWindows_is1;Inno Setup: App Path]" )
    
  FIND_PATH(WXWINDOWS_ROOT_DIR  include/wx/wx.h 
    ${WXWINDOWS_POSSIBLE_ROOT_PATHS} )
  
  ## MESSAGE("DBG found WXWINDOWS_ROOT_DIR: ${WXWINDOWS_ROOT_DIR}")
  
  ## find libs for combination of static/shared with release/debug
  SET (WXWINDOWS_POSSIBLE_LIB_PATHS
    "${WXWINDOWS_ROOT_DIR}/lib" )
  
  FIND_LIBRARY(WXWINDOWS_STATIC_LIBRARY
    NAMES wx wxmsw
    PATHS ${WXWINDOWS_POSSIBLE_LIB_PATHS}
    DOC "wxWindows static release build library" )
  
  FIND_LIBRARY(WXWINDOWS_STATIC_DEBUG_LIBRARY
    NAMES wxd wxmswd
    PATHS ${WXWINDOWS_POSSIBLE_LIB_PATHS} 
    DOC "wxWindows static debug build library" )
  
  FIND_LIBRARY(WXWINDOWS_SHARED_LIBRARY
    NAMES wxmsw24 wxmsw241 wxmsw240 wx23_2 wx22_9 
    PATHS ${WXWINDOWS_POSSIBLE_LIB_PATHS} 
    DOC "wxWindows shared release build library" )
  
  FIND_LIBRARY(WXWINDOWS_SHARED_DEBUG_LIBRARY
    NAMES wxmsw24d wxmsw241d wxmsw240d wx23_2d wx22_9d 
    PATHS ${WXWINDOWS_POSSIBLE_LIB_PATHS} 
    DOC "wxWindows shared debug build library " )
    
  
  ## if there is at least one shared lib available
  ## let user choose wether to use shared or static wxwindows libs 
  IF(WXWINDOWS_SHARED_LIBRARY OR WXWINDOWS_SHARED_DEBUG_LIBRARY)
    ## default value OFF because wxWindows MSVS default build is static
    OPTION(WXWINDOWS_USE_SHARED_LIBS
      "Use shared versions (dll) of wxWindows libraries?" OFF)
    MARK_AS_ADVANCED(WXWINDOWS_USE_SHARED_LIBS)
  ENDIF(WXWINDOWS_SHARED_LIBRARY OR WXWINDOWS_SHARED_DEBUG_LIBRARY)
  
  
  ## add system libraries wxwindows depends on
  SET(CMAKE_WXWINDOWS_LIBRARIES ${CMAKE_WXWINDOWS_LIBRARIES}
    comctl32
    rpcrt4
    wsock32
##  presumably ctl3d32 is not neccesary (Jan Woetzel 07/2003)
#   ctl3d32
#    debug ${WXWINDOWS_ROOT_DIR}/lib/zlibd.lib  optimized ${WXWINDOWS_ROOT_DIR}/lib/zlibd.lib
#    debug ${WXWINDOWS_ROOT_DIR}/lib/regexd.lib optimized ${WXWINDOWS_ROOT_DIR}/lib/regexd.lib 
#    debug ${WXWINDOWS_ROOT_DIR}/lib/pngd.lib   optimized ${WXWINDOWS_ROOT_DIR}/lib/pngd.lib
#    debug ${WXWINDOWS_ROOT_DIR}/lib/jpegd.lib  optimized ${WXWINDOWS_ROOT_DIR}/lib/jpegd.lib
#    debug ${WXWINDOWS_ROOT_DIR}/lib/tiffd.lib  optimized ${WXWINDOWS_ROOT_DIR}/lib/tiff.lib
  )
  
  # JW removed option and force the develper th SET it. 
  # OPTION(WXWINDOWS_USE_GL "use wxWindows with GL support (use additional
  # opengl, glu libs)?" OFF)
  
  ## opengl/glu: (TODO/FIXME: better use FindOpenGL.cmake here 
  IF (WXWINDOWS_USE_GL)
    SET(CMAKE_WXWINDOWS_LIBRARIES ${CMAKE_WXWINDOWS_LIBRARIES}
      opengl32
      glu32
    )
  ENDIF (WXWINDOWS_USE_GL)
  
  
  ##
  ## select between use of  shared or static wxWindows lib then set libs to use
  ## for debug and optimized build.  so the user can switch between debug and
  ## release build e.g. within MS Visual Studio without running cmake with a
  ## different build directory again.
  ## 
  ## then add the build specific include dir for wx/setup.h
  ## 
  
  IF(WXWINDOWS_USE_SHARED_LIBS)
    ##MESSAGE("DBG wxWindows use shared lib selected.")
    
    ## shared: both wx (debug and release) found?
    IF(WXWINDOWS_SHARED_DEBUG_LIBRARY AND WXWINDOWS_SHARED_LIBRARY)
      ##MESSAGE("DBG wx shared: debug and optimized found.")
      SET(WXWINDOWS_LIBRARIES ${CMAKE_WXWINDOWS_LIBRARIES}
        debug     ${WXWINDOWS_SHARED_DEBUG_LIBRARY}
        optimized ${WXWINDOWS_SHARED_LIBRARY} )
      FIND_PATH(WXWINDOWS_INCLUDE_DIR_SETUPH  wx/setup.h
        ${WXWINDOWS_ROOT_DIR}/lib/mswdlld
        ${WXWINDOWS_ROOT_DIR}/lib/mswdll )
    ENDIF(WXWINDOWS_SHARED_DEBUG_LIBRARY AND WXWINDOWS_SHARED_LIBRARY)
    
    ## shared: only debug wx lib found?
    IF(WXWINDOWS_SHARED_DEBUG_LIBRARY)
      IF(NOT WXWINDOWS_SHARED_LIBRARY)
        ##MESSAGE("DBG wx shared: debug (but no optimized) found.")
        SET(WXWINDOWS_LIBRARIES ${CMAKE_WXWINDOWS_LIBRARIES}
          ${WXWINDOWS_SHARED_DEBUG_LIBRARY} )
        FIND_PATH(WXWINDOWS_INCLUDE_DIR_SETUPH  wx/setup.h
          ${WXWINDOWS_ROOT_DIR}/lib/mswdlld )
      ENDIF(NOT WXWINDOWS_SHARED_LIBRARY)
    ENDIF(WXWINDOWS_SHARED_DEBUG_LIBRARY)
    
    ## shared: only release wx lib found?
    IF(NOT WXWINDOWS_SHARED_DEBUG_LIBRARY)
      IF(WXWINDOWS_SHARED_LIBRARY)
        ##MESSAGE("DBG wx shared: optimized (but no debug) found.")
        SET(WXWINDOWS_LIBRARIES ${CMAKE_WXWINDOWS_LIBRARIES}
          ${WXWINDOWS_SHARED_DEBUG_LIBRARY} )
        FIND_PATH(WXWINDOWS_INCLUDE_DIR_SETUPH  wx/setup.h
          ${WXWINDOWS_ROOT_DIR}/lib/mswdll )
      ENDIF(WXWINDOWS_SHARED_LIBRARY)    
    ENDIF(NOT WXWINDOWS_SHARED_DEBUG_LIBRARY)
    
    ## shared: none found?
    IF(NOT WXWINDOWS_SHARED_DEBUG_LIBRARY)
      IF(NOT WXWINDOWS_SHARED_LIBRARY)
        MESSAGE(SEND_ERROR 
          "No shared wxWindows lib found, but WXWINDOWS_USE_SHARED_LIBS=${WXWINDOWS_USE_SHARED_LIBS}.")
      ENDIF(NOT WXWINDOWS_SHARED_LIBRARY)
    ENDIF(NOT WXWINDOWS_SHARED_DEBUG_LIBRARY)


  ELSE(WXWINDOWS_USE_SHARED_LIBS)
    ##MESSAGE("DBG wxWindows static lib selected.")

    ## static: both wx (debug and release) found?
    IF(WXWINDOWS_STATIC_DEBUG_LIBRARY AND WXWINDOWS_STATIC_LIBRARY)
      ##MESSAGE("DBG wx static: debug and optimized found.")
      SET(WXWINDOWS_LIBRARIES ${CMAKE_WXWINDOWS_LIBRARIES}
        debug     ${WXWINDOWS_STATIC_DEBUG_LIBRARY}
        optimized ${WXWINDOWS_STATIC_LIBRARY} )
      FIND_PATH(WXWINDOWS_INCLUDE_DIR_SETUPH  wx/setup.h
        ${WXWINDOWS_ROOT_DIR}/lib/mswd
        ${WXWINDOWS_ROOT_DIR}/lib/msw )
    ENDIF(WXWINDOWS_STATIC_DEBUG_LIBRARY AND WXWINDOWS_STATIC_LIBRARY)
    
    ## static: only debug wx lib found?
    IF(WXWINDOWS_STATIC_DEBUG_LIBRARY)
      IF(NOT WXWINDOWS_STATIC_LIBRARY)
        ##MESSAGE("DBG wx static: debug (but no optimized) found.")
        SET(WXWINDOWS_LIBRARIES ${CMAKE_WXWINDOWS_LIBRARIES}
          ${WXWINDOWS_STATIC_DEBUG_LIBRARY} )
        FIND_PATH(WXWINDOWS_INCLUDE_DIR_SETUPH  wx/setup.h
          ${WXWINDOWS_ROOT_DIR}/lib/mswd )
      ENDIF(NOT WXWINDOWS_STATIC_LIBRARY)
    ENDIF(WXWINDOWS_STATIC_DEBUG_LIBRARY)
    
    ## static: only release wx lib found?
    IF(NOT WXWINDOWS_STATIC_DEBUG_LIBRARY)
      IF(WXWINDOWS_STATIC_LIBRARY)
        ##MESSAGE("DBG wx static: optimized (but no debug) found.")
        SET(WXWINDOWS_LIBRARIES ${CMAKE_WXWINDOWS_LIBRARIES}
          ${WXWINDOWS_STATIC_DEBUG_LIBRARY} )
        FIND_PATH(WXWINDOWS_INCLUDE_DIR_SETUPH  wx/setup.h
          ${WXWINDOWS_ROOT_DIR}/lib/msw )
      ENDIF(WXWINDOWS_STATIC_LIBRARY)
    ENDIF(NOT WXWINDOWS_STATIC_DEBUG_LIBRARY)
    
    ## static: none found?
    IF(NOT WXWINDOWS_STATIC_DEBUG_LIBRARY)
      IF(NOT WXWINDOWS_STATIC_LIBRARY)
        MESSAGE(SEND_ERROR 
          "No static wxWindows lib found, but WXWINDOWS_USE_SHARED_LIBS=${WXWINDOWS_USE_SHARED_LIBS}.")
      ENDIF(NOT WXWINDOWS_STATIC_LIBRARY)
    ENDIF(NOT WXWINDOWS_STATIC_DEBUG_LIBRARY)
    
  ENDIF(WXWINDOWS_USE_SHARED_LIBS)  
  
  
  ## not neccessary in wxWindows 2.4.1 
  ## but it may fix a previous bug, see
  ## http://lists.wxwindows.org/cgi-bin/ezmlm-cgi?8:mss:37574:200305:mpdioeneabobmgjenoap
  OPTION(WXWINDOWS_SET_DEFINITIONS "Set additional defines for wxWindows" OFF)
  MARK_AS_ADVANCED(WXWINDOWS_SET_DEFINITIONS)
  IF (WXWINDOWS_SET_DEFINITIONS) 
    SET(WXWINDOWS_DEFINITIONS "-DWINVER=0x400")
  ELSE (WXWINDOWS_SET_DEFINITIONS) 
    # clear:
    SET(WXWINDOWS_DEFINITIONS "")
  ENDIF (WXWINDOWS_SET_DEFINITIONS) 
  
  
  ## Find the include directories for wxwindows
  ## the first, build specific for wx/setup.h was determined before.
  ## add inc dir for general for "wx/wx.h" 
  SET (WXWINDOWS_POSSIBLE_INCLUDE_PATHS
    "${WXWINDOWS_ROOT_DIR}/include"
  )
  FIND_PATH(WXWINDOWS_INCLUDE_DIR  wx/wx.h 
    ${WXWINDOWS_POSSIBLE_INCLUDE_PATHS}
  )  
  # append the build specific include dir for wx/setup.h:
  IF (WXWINDOWS_INCLUDE_DIR_SETUPH)
    SET(WXWINDOWS_INCLUDE_DIR ${WXWINDOWS_INCLUDE_DIR} ${WXWINDOWS_INCLUDE_DIR_SETUPH} )
  ENDIF (WXWINDOWS_INCLUDE_DIR_SETUPH)
  
  MARK_AS_ADVANCED(
    WXWINDOWS_ROOT_DIR
    WXWINDOWS_INCLUDE_DIR
    WXWINDOWS_INCLUDE_DIR_SETUPH
    WXWINDOWS_STATIC_LIBRARY
    WXWINDOWS_STATIC_DEBUG_LIBRARY
    WXWINDOWS_SHARED_LIBRARY
    WXWINDOWS_SHARED_DEBUG_LIBRARY
  )
  
  
ELSE(WIN32)
IF (UNIX) 

  ## ######################################################################
  ## 
  ## UNIX/Linux specific:
  ## 
  ## use backquoted wx-config to query and set flags and libs:
  ## 06/2003 Jan Woetzel
  ## 
  
  OPTION(WXWINDOWS_USE_SHARED_LIBS "Use shared versions (.so) of wxWindows libraries" ON)
  MARK_AS_ADVANCED(WXWINDOWS_USE_SHARED_LIBS)

  # JW removed option and force the develper th SET it. 
  # OPTION(WXWINDOWS_USE_GL "use wxWindows with GL support (use additional
  # --gl-libs for wx-config)?" OFF)
  
  # wx-config should be in your path anyhow, usually no need to set WXWIN or
  # search in ../wx or ../../wx
  FIND_PROGRAM(CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE wx-config
    $ENV{WXWIN}
    ../wx/bin
    ../../wx/bin )
  
  # check wether wx-config was found:
  IF(CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE)    

    # use shared/static wx lib?
    # remember: always link shared to use systems GL etc. libs (no static
    # linking, just link *against* static .a libs)
    IF(WXWINDOWS_USE_SHARED_LIBS)
      SET(WX_CONFIG_ARGS_LIBS "--libs")
    ELSE(WXWINDOWS_USE_SHARED_LIBS)
      SET(WX_CONFIG_ARGS_LIBS "--static --libs")
    ENDIF(WXWINDOWS_USE_SHARED_LIBS)
  
    # do we need additionial wx GL stuff like GLCanvas ?
    IF(WXWINDOWS_USE_GL)
      SET(WX_CONFIG_ARGS_LIBS "${WX_CONFIG_ARGS_LIBS} --gl-libs" )
    ENDIF(WXWINDOWS_USE_GL)
    ##MESSAGE("DBG: WX_CONFIG_ARGS_LIBS=${WX_CONFIG_ARGS_LIBS}===")
    
    # set CXXFLAGS to be fed into CMAKE_CXX_FLAGS by the user:
    SET(CMAKE_WXWINDOWS_CXX_FLAGS "`${CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE} --cxxflags`")
    ##MESSAGE("DBG: for compilation:
    ##CMAKE_WXWINDOWS_CXX_FLAGS=${CMAKE_WXWINDOWS_CXX_FLAGS}===")

    # keep the back-quoted string for clarity
    SET(WXWINDOWS_LIBRARIES "`${CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE} ${WX_CONFIG_ARGS_LIBS}`")
    ##MESSAGE("DBG2: for linking:
    ##WXWINDOWS_LIBRARIES=${WXWINDOWS_LIBRARIES}===")
    
    # evaluate wx-config output to separate linker flags and linkdirs for
    # rpath:
    EXEC_PROGRAM(${CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE}
      ARGS ${WX_CONFIG_ARGS_LIBS}
      OUTPUT_VARIABLE WX_CONFIG_LIBS )
  
    ## extract linkdirs (-L) for rpath
    ## use regular expression to match wildcard equivalent "-L*<endchar>"
    ## with <endchar> is a space or a semicolon
    STRING(REGEX MATCHALL "[-][L]([^ ;])+" WXWINDOWS_LINK_DIRECTORIES_WITH_PREFIX ${WX_CONFIG_LIBS} )
    #MESSAGE("DBG  WXWINDOWS_LINK_DIRECTORIES_WITH_PREFIX=${WXWINDOWS_LINK_DIRECTORIES_WITH_PREFIX}")
    
    ## remove prefix -L because we need the pure directory for LINK_DIRECTORIES
    ## replace -L by ; because the separator seems to be lost otherwise (bug or
    ## feature?)
    IF(WXWINDOWS_LINK_DIRECTORIES_WITH_PREFIX)
      STRING(REGEX REPLACE "[-][L]" ";" WXWINDOWS_LINK_DIRECTORIES ${WXWINDOWS_LINK_DIRECTORIES_WITH_PREFIX} )
      #MESSAGE("DBG  WXWINDOWS_LINK_DIRECTORIES=${WXWINDOWS_LINK_DIRECTORIES}")
    ENDIF(WXWINDOWS_LINK_DIRECTORIES_WITH_PREFIX)

    
    ## replace space separated string by semicolon separated vector to make it
    ## work with LINK_DIRECTORIES
    SEPARATE_ARGUMENTS(WXWINDOWS_LINK_DIRECTORIES)
    
    MARK_AS_ADVANCED(
      CMAKE_WXWINDOWS_CXX_FLAGS
      WXWINDOWS_INCLUDE_DIR
      WXWINDOWS_LIBRARIES
      CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE
    )
  
  
  # we really need wx-config...
  ELSE(CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE)    
    MESSAGE(SEND_ERROR "Cannot find wx-config anywhere on the system. Please put the file into your path or specify it in CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE.")
    MARK_AS_ADVANCED(CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE)
  ENDIF(CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE)

  
  
ELSE(UNIX)
  MESSAGE(SEND_ERROR "FindwxWindows.cmake:  Platform unknown/unsupported by FindwxWindows.cmake. It's neither WIN32 nor UNIX")
ENDIF(UNIX)
ENDIF(WIN32)


IF(WXWINDOWS_LIBRARIES)
  IF(WXWINDOWS_INCLUDE_DIR OR CMAKE_WXWINDOWS_CXX_FLAGS)
    
    ## found all we need.
    SET(WXWINDOWS_FOUND 1)
    
    ## set deprecated variables for backward compatibility: 
    SET(CMAKE_WX_CAN_COMPILE   ${WXWINDOWS_FOUND})
    SET(WXWINDOWS_LIBRARY      ${WXWINDOWS_LIBRARIES})
    SET(WXWINDOWS_INCLUDE_PATH ${WXWINDOWS_INCLUDE_DIR})
    SET(CMAKE_WX_CXX_FLAGS     ${CMAKE_WXWINDOWS_CXX_FLAGS})
    
  ENDIF(WXWINDOWS_INCLUDE_DIR OR CMAKE_WXWINDOWS_CXX_FLAGS)
ENDIF(WXWINDOWS_LIBRARIES)
