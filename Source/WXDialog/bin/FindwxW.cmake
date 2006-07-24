
MESSAGE("${CMAKE_CURRENT_FILE} is deprecated, please use FindwxWidgets.cmake instead.")

#
# FindwxW.cmake
# v1.01 2005-05-27
# Jerry Fath 2005
# Based on work by Jorgen Bodde
#
# This module finds if wxWindows is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  WXWIDGETS_LINK_DIRECTORIES = link directories, useful for rpath on Unix
#  WXWIDGETS_LIBRARIES       = all the wxWindows libraries ( and  linker flags on unix )
#  WXWIDGETS_CXX_FLAGS        = compiler flags for building wxWindows
#  WXWIDGETS_INCLUDE_DIR    = all include path of wxWindows
#  WXWIDGETS_DEFINITIONS  = all flags of wxWindows


# NOTE: This module REQUIRES that an environment variable named WXWIN
# be set to the base wxWidgets installation directory.
# Under Unix, you must also set and environment variable named WXWINCFG
# to the name of the active build subdirectory (eg buildgtk).
#

# These variable should be set before including this module
#
# WXWIDGETS_FILE_VERSION (26)
# WXWIDGETS_VERSION (2.6)
#

#
# Set WX_USE vars in CMakeLists if you don't want to default to all off
#
SET(WXWIDGETS_USE_DEBUG ${WXW_USE_DEBUG} CACHE BOOL "Use Debug versions of wxWindows libraries")
SET(WXWIDGETS_USE_UNICODE ${WXW_USE_UNICODE} CACHE BOOL "Use Unicode versions of wxWindows libraries")
SET(WXWIDGETS_USE_SHARED ${WXW_USE_SHARED} CACHE BOOL "Use shared versions of wxWindows libraries")
SET(WXWIDGETS_USE_UNIV ${WXW_USE_UNIV} CACHE BOOL "Use Universal versions of wxWindows libraries")
SET(WXWIDGETS_USE_MONO ${WXW_USE_MONO} CACHE BOOL "Use monolithic versions of wxWindows libraries")

#
# Style can be non-Win32 even under Windows when using MinGW
#
IF(WIN32)
  SET(WIN32_STYLE_FIND 1)
ENDIF(WIN32)
IF(MINGW)
  SET(WIN32_STYLE_FIND 0)
  SET(UNIX_STYLE_FIND 1)
ENDIF(MINGW)
IF(UNIX)
  SET(UNIX_STYLE_FIND 1)
ENDIF(UNIX)

#
# Versions should be set before this file is included.  If not,
# default to 2.6
#
IF(WXW_FILE_VERSION)
  SET( WXVERSIONSUFFIX ${WXW_FILE_VERSION})
ELSE(WXW_FILE_VERSION)
  SET( WXVERSIONSUFFIX "26")
ENDIF(WXW_FILE_VERSION)

IF(WXW_VERSION)
  SET( WXVERSION ${WXW_VERSION})
ELSE(WXW_VERSION)
  SET( WXVERSION "2.6")
ENDIF(WXW_VERSION)

#
# Find the base wxWidgets install path
# NOTE: WXWIN must be set
#
FIND_PATH( WXWIDGETS_INSTALL_PATH
   wx-config.in
   $ENV{WXWIN}
)
IF(WXWIDGETS_INSTALL_PATH STREQUAL "WXWIDGETS_INSTALL_PATH-NOTFOUND")
    MESSAGE( FATAL_ERROR "FATAL_ERROR: wx-config.in NOT found.  Set WXWIN")
ENDIF(WXWIDGETS_INSTALL_PATH STREQUAL "WXWIDGETS_INSTALL_PATH-NOTFOUND")
SET(WXWIDGETS_CONFIG_PATH ${WXWIDGETS_INSTALL_PATH})

#
# Find library path (platform specific)
#
IF( WIN32_STYLE_FIND )

    IF( WXWIDGETS_USE_SHARED )
        SET(WXWIDGETS_LIB_PATH "${WXWIDGETS_INSTALL_PATH}/lib/vc_dll" )
    ELSE( WXWIDGETS_USE_SHARED )
        SET(WXWIDGETS_LIB_PATH "${WXWIDGETS_INSTALL_PATH}/lib/vc_lib" )
    ENDIF( WXWIDGETS_USE_SHARED )
    SET( WXWIDGETS_LINK_DIRECTORIES ${WXWIDGETS_LIB_PATH} )

ELSE( WIN32_STYLE_FIND )
  IF (UNIX_STYLE_FIND) 

    #
    # Unix uses a config specific directory under the install path
    # specified in the WXWINCFG environment variable.
    #
    SET(WXWIDGETS_CONFIG_PATH "${WXWIDGETS_INSTALL_PATH}/$ENV{WXWINCFG}")

    #
    # Check wether wx-config can be found
    #
    SET(WX_WXCONFIG_EXECUTABLE "${WXWIDGETS_CONFIG_PATH}/wx-config" )
    IF(NOT EXISTS ${WX_WXCONFIG_EXECUTABLE})    
      # we really need wx-config...
      MESSAGE(FATAL_ERROR "FATAL_ERROR: Cannot find wx-config. Set WXWIN and WXWINCFG environment variables.")
    ENDIF(NOT EXISTS ${WX_WXCONFIG_EXECUTABLE})    

    SET(WXWIDGETS_LIB_PATH "${WXWIDGETS_CONFIG_PATH}/lib" )
    SET( WXWIDGETS_LINK_DIRECTORIES ${WXWIDGETS_LIB_PATH} )

  ELSE(UNIX_STYLE_FIND)
    MESSAGE(FATAL_ERROR "FATAL_ERROR: FindwxWindows.cmake:  Platform unsupported by FindwxW.cmake. It's neither WIN32 nor UNIX")
  ENDIF(UNIX_STYLE_FIND)
ENDIF( WIN32_STYLE_FIND )

#
# Check options against installed libraries
#
SET(WX_CONFIG_ARGS "")
SET( LIBRARYWANT  "${WXWIDGETS_LIB_PATH}/msw" )

IF( WXWIDGETS_USE_UNIV )
    SET( LIBRARYWANT "${LIBRARYWANT}univ")
    SET( PFUNIV  "univ" )
    SET(WX_CONFIG_ARGS "${WX_CONFIG_ARGS} --universal=yes")
ELSE( WXWIDGETS_USE_UNIV )
    SET(WX_CONFIG_ARGS "${WX_CONFIG_ARGS} --universal=no")
ENDIF( WXWIDGETS_USE_UNIV )

IF( WXWIDGETS_USE_UNICODE )
    SET( LIBRARYWANT "${LIBRARYWANT}u" )
    SET( PFUNICODE "u" )
    SET(WX_CONFIG_ARGS "${WX_CONFIG_ARGS} --unicode=yes")
ELSE( WXWIDGETS_USE_UNICODE )
    SET(WX_CONFIG_ARGS "${WX_CONFIG_ARGS} --unicode=no")
ENDIF( WXWIDGETS_USE_UNICODE )

IF( WXWIDGETS_USE_DEBUG )
    SET( LIBRARYWANT "${LIBRARYWANT}d" )
    SET( PFDEBUG "d" )
    SET(WX_CONFIG_ARGS "${WX_CONFIG_ARGS} --debug=yes")
ELSE( WXWIDGETS_USE_DEBUG )
    SET( PFDEBUG "" )
    SET(WX_CONFIG_ARGS "${WX_CONFIG_ARGS} --debug=no")
ENDIF( WXWIDGETS_USE_DEBUG )

IF( WXWIDGETS_USE_SHARED )
    SET(WX_CONFIG_ARGS "${WX_CONFIG_ARGS} --static=no")
ELSE( WXWIDGETS_USE_SHARED )
    SET(WX_CONFIG_ARGS "${WX_CONFIG_ARGS} --static=yes")
ENDIF( WXWIDGETS_USE_SHARED )

# Not sure how to check this yet!!!!!
IF( WXWIDGETS_USE_MONO )
    SET(WX_CONFIG_ARGS "${WX_CONFIG_ARGS}")
ELSE( WXWIDGETS_USE_MONO )
    SET(WX_CONFIG_ARGS "${WX_CONFIG_ARGS}")
ENDIF( WXWIDGETS_USE_MONO )

SET(WX_CONFIG_ARGS_LIBS "${WX_CONFIG_ARGS} --libs")

#
# Platform specific method for testing installed wx Builds
#
IF( WIN32_STYLE_FIND )

    IF( NOT EXISTS "${LIBRARYWANT}/wx/setup.h" )
        MESSAGE( SEND_ERROR "ERROR: WXWidgets config NOT found ${WX_CONFIG_ARGS}")
    ENDIF( NOT EXISTS "${LIBRARYWANT}/wx/setup.h" )

ELSE( WIN32_STYLE_FIND )

    EXEC_PROGRAM(${WX_WXCONFIG_EXECUTABLE}
        ARGS ${WX_CONFIG_ARGS_LIBS}
        OUTPUT_VARIABLE WXWIDGETS_LIBRARIES
        RETURN_VALUE BAD_WXCONFIG )
    IF(BAD_WXCONFIG)
        MESSAGE( SEND_ERROR "ERROR: WXWidgets config NOT found ${WX_CONFIG_ARGS}")
    ENDIF(BAD_WXCONFIG)

ENDIF( WIN32_STYLE_FIND )

#
#  Get compiler flags
#
IF( WIN32_STYLE_FIND )
    # Not used for Windows
    SET( WXWIDGETS_CXX_FLAGS "" )
ELSE( WIN32_STYLE_FIND )
    #
    # Get CXXFLAGS from wx-config
    #
    EXEC_PROGRAM(${WX_WXCONFIG_EXECUTABLE}
      ARGS "--cxxflags"
      OUTPUT_VARIABLE WXWIDGETS_CXX_FLAGS
      RETURN_VALUE BAD_WXCONFIG )
    IF (BAD_WXCONFIG)
      MESSAGE( SEND_ERROR "ERROR: wx-config --cxxflags returned an error")
    ENDIF (BAD_WXCONFIG)
ENDIF( WIN32_STYLE_FIND )

#
# Find include directories (platform specific)
#
IF( WIN32_STYLE_FIND )
    #
    # Set non-build specific include directories
    #
    SET ( WX_INCLUDE_PATH ${WXWIDGETS_INSTALL_PATH}/include )
    SET ( WXWIDGETS_INCLUDE_DIR ${WX_INCLUDE_PATH} )
    SET( WXWIDGETS_INCLUDE_DIR ${WXWIDGETS_INCLUDE_DIR} ${WXWIDGETS_INCLUDE_DIR}/../contrib/include )
    #
    # Append the build specific include dir for wx/setup.h:
    #
    IF ( EXISTS ${LIBRARYWANT}/wx/setup.h )
        SET( WX_INCLUDE_LIB_PATH ${LIBRARYWANT})
        SET( WXWIDGETS_INCLUDE_DIR  ${WXWIDGETS_INCLUDE_DIR}  ${LIBRARYWANT} )
    ELSE ( EXISTS ${LIBRARYWANT}/wx/setup.h )
        SET( WX_INCLUDE_LIB_PATH, "")
        MESSAGE(SEND_ERROR "ERROR: Can't find ${LIBRARYWANT}/wx/setup.h")
    ENDIF ( EXISTS ${LIBRARYWANT}/wx/setup.h )
ELSE( WIN32_STYLE_FIND )

    # Pull out -I options
    # evaluate wx-config output to separate include dirs
    SET(WX_INCLUDE_DIR ${WXWIDGETS_CXX_FLAGS})
    # extract include dirs (-I)
    # use regular expression to match wildcard equivalent "-I*<endchar>"
    # with <endchar> is a space or a semicolon
    STRING(REGEX MATCHALL "[-][I]([^ ;])+" WX_INCLUDE_DIRS_WITH_PREFIX "${WX_INCLUDE_DIR}" )
    # remove prefix -I because we need the pure directory
    IF(WX_INCLUDE_DIRS_WITH_PREFIX)
      STRING(REGEX REPLACE "[-][I]" ";" WXWIDGETS_INCLUDE_DIR ${WX_INCLUDE_DIRS_WITH_PREFIX} )
    ENDIF(WX_INCLUDE_DIRS_WITH_PREFIX)
    # replace space separated string by semicolon separated vector
    SEPARATE_ARGUMENTS(WXWIDGETS_INCLUDE_DIR)
    # Remove the -I options from the CXX_FLAGS no need to duplicate
    STRING(REGEX REPLACE "[-][I]([^ ;])+" "" WXWIDGETS_CXX_FLAGS ${WXWIDGETS_CXX_FLAGS} )

    #
    # Find XWindows
    #
    IF( NOT CYGWIN OR MINGW )
        INCLUDE( ${CMAKE_ROOT}/Modules/FindX11.cmake )
        SET( WXWIDGETS_INCLUDE_DIR ${WXWIDGETS_INCLUDE_DIR}  ${X11_INCLUDE_DIR} )
    ENDIF( NOT CYGWIN OR MINGW )
ENDIF( WIN32_STYLE_FIND )

#
# Find library list (platform specific)
#
IF( WIN32_STYLE_FIND )
    #
    #Misc vars used to build lib names
    #
    SET( PFLIBEXT ${CMAKE_STATIC_LIBRARY_SUFFIX} )
    SET( WXPF "${WXPF}${WXVERSIONSUFFIX}" )
    SET( PFVERSION ${WXVERSIONSUFFIX} )
    SET( PFCOMPILER "" )

    #
    # Build the libraries list
    #
    SET( WXWIDGETS_LIBRARIES "" )
    SET( WXWIDGETS_MULTI_LIBRARIES "" )
    SET( WXWIDGETS_MONO_LIBRARIES "" )

    SET(WX_LIBRARIES ${WX_LIBRARIES} comctl32${PFLIBEXT} wsock32${PFLIBEXT} rpcrt4${PFLIBEXT})
    #MESSAGE( "WX_LIBRARIES: ${WX_LIBRARIES}")

    #start filling library string with needed libraries for the choosen configuration.
    SET( WXWIDGETS_LIBRARIES ${WXWIDGETS_LIBRARIES} ${WX_LIBRARIES} )

    SET( WXPF "${PFVERSION}${PFUNICODE}${PFDEBUG}" )
    SET(WXWIDGETS_MULTI_LIBRARIES ${WXWIDGETS_MULTI_LIBRARIES}
        wxbase${WXPF}${PFCOMPILER}${PFLIBEXT}
        wxbase${WXPF}_net${PFCOMPILER}${PFLIBEXT}  wxbase${WXPF}_xml${PFCOMPILER}${PFLIBEXT}
    )

    SET( WXPF "${PFVERSION}${PFUNICODE}${PFDEBUG}" )
    SET(WXWIDGETS_MULTI_LIBRARIES ${WXWIDGETS_MULTI_LIBRARIES}
        wxmsw${WXPF}_adv${PFCOMPILER}${PFLIBEXT}
        wxmsw${WXPF}_core${PFCOMPILER}${PFLIBEXT}
        wxmsw${WXPF}_html${PFCOMPILER}${PFLIBEXT}
        wxmsw${WXPF}_xrc${PFCOMPILER}${PFLIBEXT}
#        wxmsw${WXPF}_media${PFCOMPILER}${PFLIBEXT}
    )

    IF( NOT WXWIDGETS_USE_SHARED )
        SET( WXPF "${PFVERSION}${PFUNICODE}${PFDEBUG}" )
        # ODBC  is not needed
        #SET(WXWIDGETS_MULTI_LIBRARIES ${WXWIDGETS_MULTI_LIBRARIES} wxbase${WXPF}_odbc${PFCOMPILER}${PFLIBEXT} )

        SET( WXPF "${PFVERSION}${PFUNICODE}${PFDEBUG}" )
        #SET(WXWIDGETS_MULTI_LIBRARIES ${WXWIDGETS_MULTI_LIBRARIES}
        #    wxmsw${WXPF}_dbgrid${PFLIBEXT}
        #    wxmsw${WXPF}_gl${PFCOMPILER}${PFLIBEXT}
        #)
    ENDIF( NOT WXWIDGETS_USE_SHARED )

    IF ( WXWIDGETS_USE_UNIV )
        SET( WXPF "${PFUNIV}${PFVERSION}${PFUNICODE}${PFDEBUG}" )
        SET(WXWIDGETS_MULTI_LIBRARIES ${WXWIDGETS_MULTI_LIBRARIES}
            wxmsw${WXPF}_core${PFCOMPILER}${PFLIBEXT}
        )
    ENDIF ( WXWIDGETS_USE_UNIV )

    SET(WXWIDGETS_MONO_LIBRARIES ${WXWIDGETS_MONO_LIBRARIES}
        wxmsw${WXPF}${PFCOMPILER}${PFLIBEXT}
    )

    IF ( WXWIDGETS_USE_MONO )
        SET(WXWIDGETS_LIBRARIES ${WXWIDGETS_LIBRARIES} ${WXWIDGETS_MONO_LIBRARIES})
    ELSE ( WXWIDGETS_USE_MONO )
        SET(WXWIDGETS_LIBRARIES ${WXWIDGETS_LIBRARIES} ${WXWIDGETS_MULTI_LIBRARIES})
    ENDIF ( WXWIDGETS_USE_MONO )

    SET( WXPF "${PFUNICODE}${PFDEBUG}" )
    SET(WXWIDGETS_LIBRARIES ${WXWIDGETS_LIBRARIES}
        wxregex${WXPF}.lib
    )

    SET( WXPF "${PFDEBUG}" )
        SET(WXWIDGETS_LIBRARIES ${WXWIDGETS_LIBRARIES}
            wxexpat${WXPF}.lib
            wxpng${WXPF}.lib
            wxtiff${WXPF}.lib
            wxjpeg${WXPF}.lib
            wxzlib${WXPF}.lib
        )

ELSE( WIN32_STYLE_FIND )
    #
    # Get Library list from wx-config
    #
    # do we need additionial wx GL stuff like GLCanvas ?
    IF(WXWIDGETS_USE_GL)
      SET(WX_CONFIG_ARGS_LIBS "${WX_CONFIG_ARGS_LIBS} --gl-libs" )
    ENDIF(WXWIDGETS_USE_GL)
      
    EXEC_PROGRAM(${WX_WXCONFIG_EXECUTABLE}
      ARGS ${WX_CONFIG_ARGS_LIBS}
      OUTPUT_VARIABLE WXWIDGETS_LIBRARIES
      RETURN_VALUE BAD_WXCONFIG )
    IF (BAD_WXCONFIG)
      MESSAGE( SEND_ERROR "ERROR: Specified WXWidgets config NOT found")
    ENDIF (BAD_WXCONFIG)
      
    # Pull out the -L options
    # evaluate wx-config output to separate linker flags and linkdirs for
    # rpath:
    SET(WX_CONFIG_LIBS ${WXWIDGETS_LIBRARIES})
    # extract linkdirs (-L) for rpath
    # use regular expression to match wildcard equivalent "-L*<endchar>"
    # with <endchar> is a space or a semicolon
    STRING(REGEX MATCHALL "[-][L]([^ ;])+" WX_LINK_DIRECTORIES_WITH_PREFIX "${WX_CONFIG_LIBS}" )
    #MESSAGE("DBG  WX_LINK_DIRECTORIES_WITH_PREFIX=${WX_LINK_DIRECTORIES_WITH_PREFIX}")
    # remove prefix -L because we need the pure directory for LINK_DIRECTORIES
    # replace -L by ; because the separator seems to be lost otherwise (bug or
    # feature?)
    IF(WX_LINK_DIRECTORIES_WITH_PREFIX)
      STRING(REGEX REPLACE "[-][L]" ";" WXWIDGETS_LINK_DIRECTORIES ${WX_LINK_DIRECTORIES_WITH_PREFIX} )
      #MESSAGE("DBG  WXWIDGETS_LINK_DIRECTORIES=${WXWIDGETS_LINK_DIRECTORIES}")
    ENDIF(WX_LINK_DIRECTORIES_WITH_PREFIX)
    # replace space separated string by semicolon separated vector to make it
    # work with LINK_DIRECTORIES
    SEPARATE_ARGUMENTS(WXWIDGETS_LINK_DIRECTORIES)
    # Remove -L options from lib list, no need to duplicate
    STRING(REGEX REPLACE "[-][L]([^ ;])+" "" WXWIDGETS_LIBRARIES ${WXWIDGETS_LIBRARIES} )


ENDIF( WIN32_STYLE_FIND )


#
# Find wx.rc
#
SET( WXWIDGETS_RC ${WXWIDGETS_INSTALL_PATH}/include/wx/msw/wx.rc )
IF( NOT WIN32_STYLE_FIND )
    IF( NOT CYGWIN OR MINGW )
        SET( WXWIDGETS_RC "" )
    ENDIF( NOT CYGWIN OR MINGW )
ENDIF( NOT WIN32_STYLE_FIND )

#
# Set preprocessor defs
#
STRING(TOUPPER ${CMAKE_SYSTEM_NAME} WX_SYSNAME)
SET( WXWIDGETS_DEFINITIONS ${WXWIDGETS_DEFINITIONS} "-D${WX_SYSNAME}" )

IF(WIN32_STYLE_FIND)
    SET( WXWIDGETS_DEFINITIONS ${WXWIDGETS_DEFINITIONS} -DWIN32 -D__WXMSW__  )
ELSE(WIN32_STYLE_FIND)
    IF( CYGWIN )
        SET( WXWIDGETS_DEFINITIONS ${WXWIDGETS_DEFINITIONS} -D__GNUWIN32__  -O2 -D_WIN32_IE=0x400 -MMD -Wall  )
    ENDIF( CYGWIN )
ENDIF(WIN32_STYLE_FIND)

IF( WXWIDGETS_USE_SHARED )
    SET( WXWIDGETS_DEFINITIONS ${WXWIDGETS_DEFINITIONS} -DWXUSINGDLL  )
ENDIF( WXWIDGETS_USE_SHARED )

IF( WXWIDGETS_USE_DEBUG )
    SET( WXWIDGETS_DEFINITIONS ${WXWIDGETS_DEFINITIONS} -D_DEBUG_  -D__WXDEBUG__  -DWXDEBUG=1 )
ENDIF( WXWIDGETS_USE_DEBUG )

IF ( WXWIDGETS_USE_UNICODE )
    SET( WXWIDGETS_DEFINITIONS ${WXWIDGETS_DEFINITIONS} -DwxUSE_UNICODE  )
ENDIF ( WXWIDGETS_USE_UNICODE )

#
# Set the WXWIDGETS_FOUND var
#
IF(WXWIDGETS_LIBRARIES AND WXWIDGETS_INCLUDE_DIR)
    SET(WXWIDGETS_FOUND 1)
ENDIF(WXWIDGETS_LIBRARIES AND WXWIDGETS_INCLUDE_DIR)

#
# Mark advanced vars
#
MARK_AS_ADVANCED(WXWIDGETS_INSTALL_PATH)

#
# Log results
#
FILE( WRITE "${PROJECT_BINARY_DIR}/findwxw.log" "WXWIDGETS_FOUND: ${WXWIDGETS_FOUND}\n" )
FILE( APPEND "${PROJECT_BINARY_DIR}/findwxw.log" "WXWIDGETS_LINK_DIRECTORIES: ${WXWIDGETS_LINK_DIRECTORIES}\n" )
FILE( APPEND "${PROJECT_BINARY_DIR}/findwxw.log" "WXWIDGETS_LIBRARIES: ${WXWIDGETS_LIBRARIES}\n" )
FILE( APPEND "${PROJECT_BINARY_DIR}/findwxw.log" "WXWIDGETS_CXX_FLAGS: ${WXWIDGETS_CXX_FLAGS}\n" )
FILE( APPEND "${PROJECT_BINARY_DIR}/findwxw.log" "WXWIDGETS_INCLUDE_DIR: ${WXWIDGETS_INCLUDE_DIR}\n" )
FILE( APPEND "${PROJECT_BINARY_DIR}/findwxw.log" "WXWIDGETS_DEFINITIONS: ${WXWIDGETS_DEFINITIONS}\n" )
