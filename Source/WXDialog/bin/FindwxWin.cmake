##---------------------------------------------------------------------------
## $RCSfile$
## $Source$
## $Revision$
## $Date$
##---------------------------------------------------------------------------
## Author:      Jorgen Bodde
## Copyright:   (c) Jorgen Bodde
## License:     wxWidgets License
##---------------------------------------------------------------------------
##
## This module finds if wxWindows 2.5.x / 2.6.x is installed and determines 
## where the include files and seperate libraries are. It also determines what 
## the name of the library is. 
##
## WARNING: Monolithic build support is not present here (it is not desired
## by wxWidgets anyway). Also the shared DLL option is not yet present.
##
## Current working combinations are;
##      - Static debug and release libs 
##      - Static unicode debug and release libs 
##      - Static universal unicode debug and release libs 
##      - Static unicode universal unicode debug and release libs 
##
##
## The linux part of this CMake file is unaltered and will always use because of 
## 'wx-config'. The linux part will not use the WX_LIB_IGNORE vars because of 
## the way wxWidgets is built and configured.
##
## This code sets the following variables:
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

IF( WIN32 )

    ##
    ## Find root path of wxWidgets, either in the registry or as environment var
    ##

    SET (WXWINDOWS_POSSIBLE_ROOT_PATHS
        $ENV{WXWIN}
        "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\wxWindows_is1;Inno Setup: App Path]" )

    FIND_PATH(WXWIDGETS_PATH_ROOT  include/wx/wx.h
        ${WXWINDOWS_POSSIBLE_ROOT_PATHS} )
        
    SET(WXWINDOWS_FOUND 0)
    IF(NOT WXWIDGETS_PATH_ROOT)
        MESSAGE(SEND_ERROR "wxWidgets root is not found. Please point out the wxWidgets root using the WXWIN environment var")        
    ENDIF(NOT WXWIDGETS_PATH_ROOT)

    OPTION(WX_USE_DEBUG_ONLY "Only attempt to setup the debug libraries, ignore the release libraries." ON)
      
    ## Find a multi library path, this is the path where the seperate libs are located
    SET(WXMSW_SETUPH "wx/setup.h")

    SET(WXDEBUG_POSTFIX "")
    IF(WX_USE_DEBUG_ONLY)
        SET(WXDEBUG_POSTFIX "d")
    ENDIF(WX_USE_DEBUG_ONLY)

    ## Try the VS compilation path
    SET(WXWINDOWS_POSSIBLE_SETUPH 
        "${WXWIDGETS_PATH_ROOT}/lib/vc_lib/msw${WXDEBUG_POSTFIX}/${WXMSW_SETUPH}")  
        
    ## Try borland and VS7 path
    IF(EXISTS ${WXWINDOWS_POSSIBLE_SETUPH})
        SET(WXWINDOWS_POSSIBLE_MULTILIB_PATHS 
            "${WXWIDGETS_PATH_ROOT}/lib/vc_lib")
    ELSE(EXISTS ${WXWINDOWS_POSSIBLE_SETUPH})
        ## Try the borland path
        SET(WXWINDOWS_POSSIBLE_SETUPH
            "${WXWIDGETS_PATH_ROOT}/lib/bcc_lib/msw${WXDEBUG_POSTFIX}/${WXMSW_SETUPH}") 
        IF(EXISTS ${WXWINDOWS_POSSIBLE_SETUPH})
            SET(WXWINDOWS_POSSIBLE_MULTILIB_PATHS 
                "${WXWIDGETS_PATH_ROOT}/lib/bcc_lib")
        ELSE(EXISTS ${WXWINDOWS_POSSIBLE_SETUPH})
            MESSAGE(SEND_ERROR "wxWidgets libraries are not found.")        
        ENDIF(EXISTS ${WXWINDOWS_POSSIBLE_SETUPH})
    ENDIF(EXISTS ${WXWINDOWS_POSSIBLE_SETUPH})
                
    ## Ask user what config should be used, after asking, verify if it will work
    OPTION(WX_LINK_AGAINST_UNIVERSAL    "Use wxWidgets universal builds" OFF)
    OPTION(WX_LINK_AGAINST_UNICODE      "Use wxWidgets unicode builds" OFF)

    SET(WXMSW_SETUPH "wx/setup.h")

    SET(WX_RELEASE 0)
    SET(WX_DEBUG 0)
    
    SET(WX_RELEASE_UNI 0)
    SET(WX_DEBUG_UNI 0)
    
    SET(WX_RELEASE_UNIV 0)
    SET(WX_DEBUG_UNIV 0)
    
    SET(WX_RELEASE_UNIV_UNI 0)
    SET(WX_DEBUG_UNIV_UNI 0)
    
    SET(WX_FOUND_VALID_CONF 0)
    
    ## Scan for wxWidgets static multilib release
    IF(EXISTS "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/msw/${WXMSW_SETUPH}")
        SET(WX_FOUND_VALID_CONF 1)
        SET(WX_RELEASE 1)
        SET(WX_RELEASE_SETUPH ${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/msw/${WXMSW_SETUPH} )
    ENDIF(EXISTS "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/msw/${WXMSW_SETUPH}")

    ## Scan for wxWidgets static multilib debug
    IF(EXISTS "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswd/${WXMSW_SETUPH}")
        SET(WX_FOUND_VALID_CONF 1)
        SET(WX_DEBUG 1)
        SET(WX_DEBUG_SETUPH ${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswd/${WXMSW_SETUPH} )
    ENDIF(EXISTS "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswd/${WXMSW_SETUPH}")

    ## Scan for wxWidgets multilib unicode
    IF(EXISTS "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswu/${WXMSW_SETUPH}")
        SET(WX_FOUND_VALID_CONF 1)
        SET(WX_RELEASE_UNI 1)
        SET(WX_RELEASE_UNI_SETUPH ${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswu/${WXMSW_SETUPH} )
    ENDIF(EXISTS "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswu/${WXMSW_SETUPH}")

    ## Scan for wxWidgets multilib unicode debug
    IF(EXISTS "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswud/${WXMSW_SETUPH}")
        SET(WX_FOUND_VALID_CONF 1)
        SET(WX_DEBUG_UNI 1)
        SET(WX_DEBUG_UNI_SETUPH ${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswu/${WXMSW_SETUPH} )
    ENDIF(EXISTS "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswud/${WXMSW_SETUPH}")

    ## Scan for wxWidgets multilib universal
    IF(EXISTS "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswuniv/${WXMSW_SETUPH}")
        SET(WX_FOUND_VALID_CONF 1)
        SET(WX_RELEASE_UNIV 1)
        SET(WX_RELEASE_UNIV_SETUPH ${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswuniv/${WXMSW_SETUPH} )
    ENDIF(EXISTS "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswuniv/${WXMSW_SETUPH}")

    ## Scan for wxWidgets multilib universal debug
    IF(EXISTS "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswunivd/${WXMSW_SETUPH}")
        SET(WX_FOUND_VALID_CONF 1)
        SET(WX_DEBUG_UNIV 1)
        SET(WX_DEBUG_UNIV_SETUPH ${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswunivd/${WXMSW_SETUPH} )
    ENDIF(EXISTS "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswunivd/${WXMSW_SETUPH}")

    ## Scan for wxWidgets multilib universal unicode 
    IF(EXISTS "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswunivu/${WXMSW_SETUPH}")
        SET(WX_FOUND_VALID_CONF 1)
        SET(WX_RELEASE_UNIV_UNI 1)
        SET(WX_RELEASE_UNIV_UNI_SETUPH ${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswunivu/${WXMSW_SETUPH} )
    ENDIF(EXISTS "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswunivu/${WXMSW_SETUPH}")

    ## Scan for wxWidgets multilib universal unicode debug
    IF(EXISTS "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswunivud/${WXMSW_SETUPH}")
        SET(WX_FOUND_VALID_CONF 1)
        SET(WX_DEBUG_UNIV_UNI 1)
        SET(WX_DEBUG_UNIV_UNI_SETUPH ${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswunivud/${WXMSW_SETUPH} )
    ENDIF(EXISTS "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/mswunivud/${WXMSW_SETUPH}")

    ## If no valid config is found, mention this
    IF(NOT WX_FOUND_VALID_CONF)
        MESSAGE(SEND_ERROR "Did not found a valid setup.h for any library. Please compile your wxWidgets libraries first")
    ENDIF(NOT WX_FOUND_VALID_CONF)

    ## Search for wx version automatically
    SET (WXVERSION "")
    
    SET(WXLIBS 
        "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/wxbase25.lib"
        "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/wxbase25d.lib"
        "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/wxbase25u.lib"
        "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/wxbase25ud.lib"
        "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/wxmswuniv25_core.lib"
        "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/wxmswuniv25d_core.lib"
        "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/wxmswuniv25u_core.lib"
        "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/wxmswuniv25ud_core.lib" )
        
    FOREACH(WXLIB ${WXLIBS})
        IF(EXISTS "${WXLIB}")
            SET(WXVERSION "25")
        ENDIF(EXISTS "${WXLIB}")            
    ENDFOREACH(WXLIB ${WXLIBS})
    
    IF(NOT WXLIB)
        SET(WXLIBS 
            "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/wxbase26.lib"
            "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/wxbase26d.lib"
            "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/wxbase26u.lib"
            "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/wxbase26ud.lib"
            "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/wxmswuniv26_core.lib"
            "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/wxmswuniv26d_core.lib"
            "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/wxmswuniv26u_core.lib"
            "${WXWINDOWS_POSSIBLE_MULTILIB_PATHS}/wxmswuniv26ud_core.lib")
    
        FOREACH(WXLIB ${WXLIBS})
            IF(EXISTS "${WXLIB}")
                SET(WXVERSION "26")
            ENDIF(EXISTS "${WXLIB}")            
        ENDFOREACH(WXLIB ${WXLIBS})    
    ENDIF(NOT WXLIB)
      
    IF(NOT WXVERSION)
        MESSAGE(SEND_ERROR "There are no suitable wxWidgets libraries found (monolithic builds are no longer supported")
    ENDIF(NOT WXVERSION)
    
    ##
    ## Verify unicode universal builds
    ##
    IF(WX_LINK_AGAINST_UNIVERSAL AND WX_LINK_AGAINST_UNICODE)
        IF(NOT WX_RELEASE_UNIV AND NOT WX_DEBUG_UNIV AND NOT WX_RELEASE_UNIV_UNI AND NOT WX_DEBUG_UNIV_UNI)
            MESSAGE(SEND_ERROR "Not all wx${WXVERSION} universal unicode libraries are found (check debug and release builds)")
        ENDIF(NOT WX_RELEASE_UNIV AND NOT WX_DEBUG_UNIV AND NOT WX_RELEASE_UNIV_UNI AND NOT WX_DEBUG_UNIV_UNI)
    ELSE(WX_LINK_AGAINST_UNIVERSAL AND WX_LINK_AGAINST_UNICODE)
        ##
        ## Verify unicode builds
        ##
        IF(NOT WX_LINK_AGAINST_UNIVERSAL AND WX_LINK_AGAINST_UNICODE)
            IF(NOT WX_RELEASE_UNI AND NOT WX_DEBUG_UNI)
                MESSAGE(SEND_ERROR "Not all wx${WXVERSION} unicode libraries are found (check debug and release builds)")        
            ENDIF(NOT WX_RELEASE_UNI AND NOT WX_DEBUG_UNI)        
        ELSE(NOT WX_LINK_AGAINST_UNIVERSAL AND WX_LINK_AGAINST_UNICODE)
            ##
            ## Verify universal builds
            ##
            IF(WX_LINK_AGAINST_UNIVERSAL AND NOT WX_LINK_AGAINST_UNICODE)
                IF(NOT WX_RELEASE_UNIV AND NOT WX_DEBUG_UNIV)
                    MESSAGE(SEND_ERROR "Not all wx${WXVERSION} universal libraries are found (check debug and release builds)")        
                ENDIF(NOT WX_RELEASE_UNIV AND NOT WX_DEBUG_UNIV)        
            ELSE(WX_LINK_AGAINST_UNIVERSAL AND NOT WX_LINK_AGAINST_UNICODE)
                ##
                ## Must be normal build now
                ##
                IF(NOT WX_RELEASE AND NOT WX_DEBUG)
                    MESSAGE(SEND_ERROR "Not all wx${WXVERSION} libraries are found (check debug and release builds)")        
                ENDIF(NOT WX_RELEASE AND NOT WX_DEBUG)                                
            ENDIF(WX_LINK_AGAINST_UNIVERSAL AND NOT WX_LINK_AGAINST_UNICODE)        
        ENDIF(NOT WX_LINK_AGAINST_UNIVERSAL AND WX_LINK_AGAINST_UNICODE)
    ENDIF(WX_LINK_AGAINST_UNIVERSAL AND WX_LINK_AGAINST_UNICODE)            
           
    ## Prepare the representation for the library
    IF(WX_LINK_AGAINST_UNICODE)
        SET(WXLIB_POST "u")
    ELSE(WX_LINK_AGAINST_UNICODE)
        SET(WXLIB_POST "")    
    ENDIF(WX_LINK_AGAINST_UNICODE)
    
    ## Prepare for universal presentation
    IF(WX_LINK_AGAINST_UNIVERSAL)
        SET(WXLIB_UNIV "univ")
    ELSE(WX_LINK_AGAINST_UNIVERSAL)
        SET(WXLIB_UNIV "") 
    ENDIF(WX_LINK_AGAINST_UNIVERSAL)
                            
    FIND_PATH(WXWINDOWS_INCLUDE_DIR_SETUPH  wx/setup.h
        ${WXWIDGETS_PATH_ROOT}/lib/vc_lib/msw${WXLIB_UNIV}${WXLIB_POST}
        ${WXWIDGETS_PATH_ROOT}/lib/vc_lib/msw${WXLIB_UNIV}${WXLIB_POST}d )
                         
    ## Find setup.h belonging to multi libs
    SET(WXWINDOWS_LINK_DIRECTORIES 
        ${WXWINDOWS_POSSIBLE_MULTILIB_PATHS} )

    ## Append all multi libs
    SET (WXWINDOWS_STATIC_RELEASE_LIBS
         optimized wxmsw${WXLIB_UNIV}${WXVERSION}${WXLIB_POST}_gl.lib
         optimized wxmsw${WXLIB_UNIV}${WXVERSION}${WXLIB_POST}_xrc.lib
         optimized wxbase${WXVERSION}${WXLIB_POST}_xml.lib
         optimized wxmsw${WXLIB_UNIV}${WXVERSION}${WXLIB_POST}_qa.lib
         optimized wxmsw${WXLIB_UNIV}${WXVERSION}${WXLIB_POST}_html.lib
         optimized wxmsw${WXLIB_UNIV}${WXVERSION}${WXLIB_POST}_dbgrid.lib
         optimized wxbase${WXVERSION}${WXLIB_POST}_odbc.lib
         optimized wxmsw${WXLIB_UNIV}${WXVERSION}${WXLIB_POST}_media.lib
         optimized wxmsw${WXLIB_UNIV}${WXVERSION}${WXLIB_POST}_adv.lib
         optimized wxmsw${WXLIB_UNIV}${WXVERSION}${WXLIB_POST}_core.lib
         optimized wxbase${WXVERSION}${WXLIB_POST}_net.lib
         optimized wxbase${WXVERSION}${WXLIB_POST}.lib
         optimized wxexpat.lib
         optimized wxtiff.lib
         optimized wxjpeg.lib
         optimized wxpng.lib
         optimized wxzlib.lib
         optimized wxregex${WXLIB_POST}.lib )

    SET (WXWINDOWS_STATIC_DEBUG_LIBS
         debug wxmsw${WXLIB_UNIV}${WXVERSION}${WXLIB_POST}d_gl.lib
         debug wxmsw${WXLIB_UNIV}${WXVERSION}${WXLIB_POST}d_xrc.lib
         debug wxbase${WXVERSION}${WXLIB_POST}d_xml.lib
         debug wxmsw${WXLIB_UNIV}${WXVERSION}${WXLIB_POST}d_qa.lib
         debug wxmsw${WXLIB_UNIV}${WXVERSION}${WXLIB_POST}d_html.lib
         debug wxmsw${WXLIB_UNIV}${WXVERSION}${WXLIB_POST}d_dbgrid.lib
         debug wxbase${WXVERSION}${WXLIB_POST}d_odbc.lib
         debug wxmsw${WXLIB_UNIV}${WXVERSION}${WXLIB_POST}d_media.lib
         debug wxmsw${WXLIB_UNIV}${WXVERSION}${WXLIB_POST}d_adv.lib
         debug wxmsw${WXLIB_UNIV}${WXVERSION}${WXLIB_POST}d_core.lib
         debug wxbase${WXVERSION}${WXLIB_POST}d_net.lib
         debug wxbase${WXVERSION}${WXLIB_POST}d.lib
         debug wxexpatd.lib
         debug wxtiffd.lib
         debug wxjpegd.lib
         debug wxpngd.lib
         debug wxzlibd.lib
         debug wxregex${WXLIB_POST}d.lib )                 
    
    ## Add the common libraries to the big list
    SET(CMAKE_WXWINDOWS_COMMON_LIBRARIES 
        winmm
        comctl32
        rpcrt4
        wsock32 )

    ## Set wxWidgets library in WXWINDOWS_LIBRARIES    
    SET(WXWINDOWS_LIBRARIES 
        ${CMAKE_WXWINDOWS_COMMON_LIBRARIES}
        ${WXWINDOWS_STATIC_DEBUG_LIBS}
        ${WXWINDOWS_STATIC_RELEASE_LIBS} )
        
    ## Find include directory    
    SET (WXWINDOWS_POSSIBLE_INCLUDE_PATHS
        "${WXWIDGETS_PATH_ROOT}/include" )
        
    FIND_PATH(WXWIDGETS_INCLUDE_DIR  wx/wx.h 
        ${WXWINDOWS_POSSIBLE_INCLUDE_PATHS} )
          
    ## Blank the setup.h dir when this is not found
    IF (NOT WXWINDOWS_INCLUDE_DIR_SETUPH)
        SET(WXWINDOWS_INCLUDE_DIR_SETUPH "")
    ENDIF (NOT WXWINDOWS_INCLUDE_DIR_SETUPH)
           
    ## Set include paths
    SET(WXWINDOWS_INCLUDE_DIR 
        ${WXWIDGETS_INCLUDE_DIR} 
        ${WXWINDOWS_INCLUDE_DIR_SETUPH} )
    
    IF(WX_LINK_AGAINST_UNICODE)
        SET(WX_UNICODE_FLAGS " -DUNICODE=1" )
    ELSE(WX_LINK_AGAINST_UNICODE)        
        SET(WX_UNICODE_FLAGS "" )
    ENDIF(WX_LINK_AGAINST_UNICODE)
    
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}${WX_UNICODE_FLAGS} -D__WXDEBUG__ -D__WXMSW__ -DWINVER=0x0400 -DwxUSE_BASE=1")
        
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}${WX_UNICODE_FLAGS} -D__WXMSW__ -DWINVER=0x0400 -DwxUSE_BASE=1" )

    ## TODO: Once these tags work, this is more efficient
    ##ADD_DEFINITIONS(debug )
    ##ADD_DEFINITIONS(-D__WXMSW__ -DWINVER=0x0400 -DwxUSE_BASE=1 )

    MARK_AS_ADVANCED(
        WXWIDGETS_INCLUDE_DIR
        WXWIDGETS_PATH_MONOLIB_DEBUG
        WXWIDGETS_PATH_MONOLIB_RELEASE
        WXWIDGETS_PATH_ROOT
        WXWINDOWS_INCLUDE_DIR_SETUPH )
        
    IF(WXWINDOWS_LINK_DIRECTORIES AND WXWINDOWS_LIBRARIES AND WXWINDOWS_INCLUDE_DIR)
        SET(WXWINDOWS_FOUND 1)
    ENDIF(WXWINDOWS_LINK_DIRECTORIES AND WXWINDOWS_LIBRARIES AND WXWINDOWS_INCLUDE_DIR)
        
ELSE( WIN32 )
    
    FIND_PROGRAM( CMAKE_WX_CONFIG wx-config ../wx/bin ../../wx/bin )
    IF(CMAKE_WX_CONFIG)
      SET(WXWINDOWS_FOUND 1)
      # run the config program to get cxxflags
      EXEC_PROGRAM(${CMAKE_WX_CONFIG} ARGS --cxxflags OUTPUT_VARIABLE
        CMAKE_WX_CXX_FLAGS RETURN_VALUE RET1)
      # run the config program to get the libs
      EXEC_PROGRAM(${CMAKE_WX_CONFIG} ARGS --libs OUTPUT_VARIABLE
        WXWINDOWS_LIBRARIES_TMP RETURN_VALUE RET2)
      # for libraries break things up into a ; separated variable
      SEPARATE_ARGUMENTS(WXWINDOWS_LIBRARIES_TMP)
      SET(LAST_FRAME 0)
      # now put the stuff back into WXWINDOWS_LIBRARIES
      # but combine all the -framework foo arguments back together
      FOREACH(arg ${WXWINDOWS_LIBRARIES_TMP})
        IF(${arg} MATCHES "-framework")
          SET(LAST_FRAME 1)
        ELSE(${arg} MATCHES "-framework")
          # not a -framework argument
          IF(LAST_FRAME)
            SET(WXWINDOWS_LIBRARIES ${WXWINDOWS_LIBRARIES} "-framework ${arg}")
            SET(LAST_FRAME 0)
          ELSE(LAST_FRAME)
            SET(WXWINDOWS_LIBRARIES ${WXWINDOWS_LIBRARIES} ${arg})
          ENDIF(LAST_FRAME)
        ENDIF(${arg} MATCHES "-framework")
      ENDFOREACH(arg)
    ENDIF(CMAKE_WX_CONFIG)

    
    ## extract linkdirs (-L) for rpath
    ## use regular expression to match wildcard equivalent "-L*<endchar>"
    ## with <endchar> is a space or a semicolon
    STRING(REGEX MATCHALL "[-][L]([^ ;])+" WXWINDOWS_LINK_DIRECTORIES_WITH_PREFIX "${WX_CONFIG_LIBS}" )
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

    IF( CYGWIN OR MINGW )
        GET_FILENAME_COMPONENT(WXWINDOWS_INSTALL_PATH ${CMAKE_WX_CONFIG} PATH)
        SET( WXWINDOWS_INSTALL_PATH ${WXWINDOWS_INSTALL_PATH}/.. )
        SET( WXWINDOWS_RC ${WXWINDOWS_INSTALL_PATH}/include/wx/msw/wx.rc )
    ELSE( CYGWIN OR MINGW )
        SET( WXWINDOWS_RC "" )
    ENDIF( CYGWIN OR MINGW )
    SET( WIN32GUI "" ) 
    
    #keep debug settings as indicated by wx-config
    #SET( WXWINDOWS_DEFINITIONS ${WXWINDOWS_DEFINITIONS} "${CMAKE_WX_CXX_FLAGS}" )
    #IF( WXWINDOWS_USE_DEBUG )
    #    SET( WXWINDOWS_DEFINITIONS ${WXWINDOWS_DEFINITIONS} -ggdb )
    #ENDIF( WXWINDOWS_USE_DEBUG )

    IF( CYGWIN OR MINGW )
        IF( CYGWIN )
            SET( WXWINDOWS_DEFINITIONS ${WXWINDOWS_DEFINITIONS} -D__GNUWIN32__  -O2 -D_WIN32_IE=0x400 -MMD -Wall  )
        ENDIF( CYGWIN )
    ELSE( CYGWIN OR MINGW )
        # just in case `gtk-config --cflags`does not work
        # SET( WXWINDOWS_INCLUDE_DIR ${WXWINDOWS_INCLUDE_DIR}  /usr/include/gtk-2.0 /usr/include/gtk-2.0/include /usr/lib/gtk-2.0/include /usr/include/glib-2.0 /usr/lib/glib-2.0/include /usr/include/pango-1.0 /usr/include/atk-1.0 )
        # What about FindGTK.cmake? and what if somebody uses wxMotif?
        IF(NOT APPLE)
          SET( CMAKE_WX_CXX_FLAGS "${CMAKE_WX_CXX_FLAGS} `gtk-config --cflags`" )
          #find Xwindows
          INCLUDE( ${CMAKE_ROOT}/Modules/FindX11.cmake )
          SET( WXWINDOWS_INCLUDE_DIR ${WXWINDOWS_INCLUDE_DIR}  ${X11_INCLUDE_DIR} )
        ENDIF(NOT APPLE)
    ENDIF( CYGWIN OR MINGW )
   
ENDIF( WIN32 )

MARK_AS_ADVANCED(
    CMAKE_WX_CXX_FLAGS
)

#MESSAGE( "${WXWINDOWS_LINK_DIRECTORIES}" )
#MESSAGE( "${WXWINDOWS_LIBRARIES}" )
#MESSAGE( "CMAKE_WX_CXX_FLAGS}" )
#MESSAGE( "WXWINDOWS_INCLUDE_PATH}" )
#MESSAGE( "WXWINDOWS_INCLUDE_DIR}" )
#MESSAGE( "WXWINDOWS_DEFINITIONS}" )
