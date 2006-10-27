# - Find a wxWidgets (a.k.a., wxWindows) installation.
# This module finds if wxWidgets is installed and selects a default
# configuration to use.
#
# The following variables are searched for and set to defaults in case
# of multiple choices. Change them if the defaults are not desired:
#
#  wxWidgets_ROOT_DIR      - Base wxWidgets directory
#                            (e.g., C:/wxWidgets-2.6.3).
#  wxWidgets_LIB_DIR       - Path to wxWidgets libraries
#                            (e.g., C:/wxWidgets-2.6.3/lib/vc_lib).
#  wxWidgets_CONFIGURATION - Configuration to use
#                            (e.g., msw, mswd, mswu, mswunivud, etc.)
#  wxWidgets_USE_LIBS      - Libraries to use besides the common
#                            required ones; set to base and core by
#                            default. You couls also list them in
#                            FIND_PACKAGE(wxWidgets REQUIRED
#                                         <components>)
#
# The following are set after configuration is done:
#
#  wxWidgets_FOUND            - Set to TRUE if wxWidgets was found.
#  wxWidgets_INCLUDE_DIRS     - Include directories for WIN32
#                               i.e., where to find "wx/wx.h" and
#                               "wx/setup.h"; possibly empty for unices.
#  wxWidgets_LIBRARIES        - Path to the wxWidgets libraries.
#  wxWidgets_LIBRARY_DIRS     - compile time link dirs, useful for
#                               rpath on UNIX. Typically an empty string
#                               in WIN32 environment.
#  wxWidgets_DEFINITIONS      - Contains defines required to compile/link
#                               against WX, e.g. -DWXUSINGDLL
#  wxWidgets_CXX_FLAGS        - Include dirs and ompiler flags for
#                               unices, empty on WIN32. Esentially
#                               "`wx-config --cxxflags`".
#  wxWidgets_USE_FILE         - convenience include file
#
# Sample usage:
#
#   SET(wxWidgets_USE_LIBS base core gl net)
#   FIND_PACKAGE(wxWidgets)
#   IF(wxWidgets_FOUND)
#     INCLUDE(${wxWidgets_USE_FILE})
#     # and for each of your dependant executable/library targets:
#     TARGET_LINK_LIBRARIES(<YourTarget> ${wxWidgets_LIBRARIES})
#   ENDIF(wxWidgets_FOUND)
#
# Sample usage with monolithic wx build:
#   SET(wxWidgets_USE_LIBS msw26 expat jpeg gl png regex tiff zlib)
#   ...

# NOTES
#
# This module has been tested on the WIN32 platform with wxWidgets
# 2.6.2, 2.6.3, and 2.5.3. However, it has been designed to
# easily extend support to all possible builds, e.g., static/shared,
# debug/release, unicode, universal, multilib/monolithic, etc..
#
# If you want to use the module and your build type is not supported
# out-of-the-box, please contact me to exchange information on how
# your system is setup and I'll try to add support for it.
#
# AUTHOR
#
# Miguel A. Figueroa-Villanueva (miguelf at ieee dot org).
# Jan Woetzel (jw at mip.informatik.uni-kiel.de).
#
# Based on previous works of:
# Jan Woetzel (FindwxWindows.cmake),
# Jorgen Bodde and Jerry Fath (FindwxWin.cmake).

#
# TODO/ideas
#
# (1) Option/Setting to use all available wx libs
# In contrast to expert developer who lists the
# minimal set of required libs in wxWidgets_USE_LIBS
# there is the newbie user:
#   - who just wants to link against WX with more 'magic'
#   - doesn't know the internal structure of WX or how it was built,
#     in particular if it is monolithic or not
#   - want to link against all available WX libs
# Basically, the intent here is to mimic what wx-config would do by
# default (i.e., `wx-config --libs`).
#
# Possible solution:
#   Add a reserved keyword "std" that initializes to what wx-config
# would default to. If the user has not set the wxWidgets_USE_LIBS,
# default to "std" instead of "base core" as it is now. To implement
# "std" will basically boil down to a FOR_EACH lib-FOUND, but maybe
# checking whether a minimal set was found.
#


#
# Helper macro to control the debugging output globally.
# - NOTE: This and all the DBG_MSG calls should be removed after the
#         module stabilizes.
#
MACRO(DBG_MSG _MSG)
  # MESSAGE(STATUS "${CMAKE_CURRENT_LIST_FILE}(${CMAKE_CURRENT_LIST_LINE}): ${_MSG}")
ENDMACRO(DBG_MSG)

# for compatibility with CMake 2.4.2
# emulate wxWidgets_FIND_COMPONENTS
IF (NOT wxWidgets_FIND_COMPONENTS)
  SET(wxWidgets_FIND_COMPONENTS "")
  FOREACH(COMPONENT
      base
      core
      adv
      dbgrid
      expat
      gl
      jpeg
      html
      media
      msw msw28 msw27 msw26
      mono
      net
      odbc
      png
      qa
      regex
      tiff
      # std # no true lib/component - list for compatibility with _USE_LIBS ?
      xml
      xrc
      zlib
      )
    IF   (${wxWidgets_FIND_REQUIRED_${COMPONENT}})
      SET(wxWidgets_FIND_COMPONENTS ${wxWidgets_FIND_COMPONENTS} ${COMPONENT})
    ENDIF(${wxWidgets_FIND_REQUIRED_${COMPONENT}})
  ENDFOREACH(COMPONENT)
ENDIF (NOT wxWidgets_FIND_COMPONENTS)

DBG_MSG("wxWidgets_FIND_COMPONENTS=${wxWidgets_FIND_COMPONENTS}")

#
# Clear return values in case the module is loaded more than once.
#
SET(wxWidgets_FOUND FALSE)
#
SET(wxWidgets_INCLUDE_DIRS      "")
SET(wxWidgets_LIBRARIES        "")
SET(wxWidgets_LIBRARY_DIRS "")
SET(wxWidgets_CXX_FLAGS        "")

#=====================================================================
#=====================================================================
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


IF(WIN32_STYLE_FIND)

  # global settings for std and common wx libs
  # logic could determine _USE_MONOLITHIC automatically
  # but let the user decide for now.
  IF   (wxWidgets_USE_MONOLITHIC)
    SET(wxWidgets_STD_LIBRARIES mono )
  ELSE (wxWidgets_USE_MONOLITHIC)
    SET(wxWidgets_STD_LIBRARIES base core ) # this is default
  ENDIF(wxWidgets_USE_MONOLITHIC)

  #useful common wx libs needed by almost all components
  SET(wxWidgets_COMMON_LIBRARIES  png tiff jpeg zlib regex expat)

  #---------------------------------------------------------------------
  # WIN32: Helper MACROS
  #---------------------------------------------------------------------
  # Get filename components for a configuration. For example,
  #   if _CONFIGURATION = mswunivud, then _UNV=univ, _UCD=u _DBG=d
  #   if _CONFIGURATION = mswu,      then _UNV="",   _UCD=u _DBG=""
  #
  MACRO(WX_GET_NAME_COMPONENTS _CONFIGURATION _UNV _UCD _DBG)
    STRING(REGEX MATCH "univ" ${_UNV} "${_CONFIGURATION}")
    STRING(REGEX REPLACE "msw.*(u)[d]*$" "u" ${_UCD} "${_CONFIGURATION}")
    IF(${_UCD} STREQUAL ${_CONFIGURATION})
      SET(${_UCD} "")
    ENDIF(${_UCD} STREQUAL ${_CONFIGURATION})
    STRING(REGEX MATCH "d$" ${_DBG} "${_CONFIGURATION}")
  ENDMACRO(WX_GET_NAME_COMPONENTS)

  #
  # Find libraries associated to a configuration.
  #
  MACRO(WX_FIND_LIBS _UNV _UCD _DBG)
    DBG_MSG("m_unv = ${_UNV}")
    DBG_MSG("m_ucd = ${_UCD}")
    DBG_MSG("m_dbg = ${_DBG}")

    # Find wxWidgets common libraries
    FOREACH(LIB png tiff jpeg zlib regex expat)
      FIND_LIBRARY(WX_${LIB}${_DBG}
        NAMES
        wx${LIB}${_UCD}${_DBG} # for regex
        wx${LIB}${_DBG}
        PATHS ${WX_LIB_DIR}
        NO_DEFAULT_PATH
        )
      MARK_AS_ADVANCED(WX_${LIB}${_DBG})
    ENDFOREACH(LIB)

    # Find wxWidgets multilib base libraries
    FIND_LIBRARY(WX_base${_DBG}
      NAMES
      wxbase28${_UCD}${_DBG}
      wxbase27${_UCD}${_DBG}
      wxbase26${_UCD}${_DBG}
      wxbase25${_UCD}${_DBG}
      PATHS ${WX_LIB_DIR}
      NO_DEFAULT_PATH
      )
    MARK_AS_ADVANCED(WX_base${_DBG})
    FOREACH(LIB net odbc xml)
      FIND_LIBRARY(WX_${LIB}${_DBG}
        NAMES
        wxbase28${_UCD}${_DBG}_${LIB}
        wxbase27${_UCD}${_DBG}_${LIB}
        wxbase26${_UCD}${_DBG}_${LIB}
        wxbase25${_UCD}${_DBG}_${LIB}
        PATHS ${WX_LIB_DIR}
        NO_DEFAULT_PATH
        )
      MARK_AS_ADVANCED(WX_${LIB}${_DBG})
    ENDFOREACH(LIB)

    # Find wxWidgets monolithic library
    FIND_LIBRARY(WX_mono${_DBG}
      NAMES
      wxmsw${_UNV}28${_UCD}${_DBG}
      wxmsw${_UNV}27${_UCD}${_DBG}
      wxmsw${_UNV}26${_UCD}${_DBG}
      wxmsw${_UNV}25${_UCD}${_DBG}
      PATHS ${WX_LIB_DIR}
      NO_DEFAULT_PATH
      )
    MARK_AS_ADVANCED(WX_mono${_DBG})

    # Find wxWidgets multilib libraries
    FOREACH(LIB core adv html media xrc dbgrid gl qa)
      FIND_LIBRARY(WX_${LIB}${_DBG}
        NAMES
        wxmsw${_UNV}28${_UCD}${_DBG}_${LIB}
        wxmsw${_UNV}27${_UCD}${_DBG}_${LIB}
        wxmsw${_UNV}26${_UCD}${_DBG}_${LIB}
        wxmsw${_UNV}25${_UCD}${_DBG}_${LIB}
        PATHS ${WX_LIB_DIR}
        NO_DEFAULT_PATH
        )
      MARK_AS_ADVANCED(WX_${LIB}${_DBG})
    ENDFOREACH(LIB)
  ENDMACRO(WX_FIND_LIBS)

  #
  # Clear all library paths, so that FIND_LIBRARY refinds them.
  #
  # Clear a lib, reset its found flag, and mark as advanced.
  MACRO(WX_CLEAR_LIB _LIB)
    SET(${_LIB} "${_LIB}-NOTFOUND" CACHE FILEPATH "Cleared." FORCE)
    SET(${_LIB}_FOUND FALSE)
    MARK_AS_ADVANCED(${_LIB})
  ENDMACRO(WX_CLEAR_LIB)
  # Clear all debug or release library paths (arguments are "d" or "").
  MACRO(WX_CLEAR_ALL_LIBS _DBG)
    # Clear wxWidgets common libraries
    FOREACH(LIB png tiff jpeg zlib regex expat)
      WX_CLEAR_LIB(WX_${LIB}${_DBG})
    ENDFOREACH(LIB)

    # Clear wxWidgets multilib base libraries
    WX_CLEAR_LIB(WX_base${_DBG})
    FOREACH(LIB net odbc xml)
      WX_CLEAR_LIB(WX_${LIB}${_DBG})
    ENDFOREACH(LIB)

    # Clear wxWidgets monolithic library
    WX_CLEAR_LIB(WX_mono${_DBG})

    # Clear wxWidgets multilib libraries
    FOREACH(LIB core adv html media xrc dbgrid gl qa)
      WX_CLEAR_LIB(WX_${LIB}${_DBG})
    ENDFOREACH(LIB)
  ENDMACRO(WX_CLEAR_ALL_LIBS)
  # Clear all wxWidgets debug libraries.
  MACRO(WX_CLEAR_ALL_DBG_LIBS)
    WX_CLEAR_ALL_LIBS("d")
  ENDMACRO(WX_CLEAR_ALL_DBG_LIBS)
  # Clear all wxWidgets release libraries.
  MACRO(WX_CLEAR_ALL_REL_LIBS)
    WX_CLEAR_ALL_LIBS("")
  ENDMACRO(WX_CLEAR_ALL_REL_LIBS)

  #
  # Set the wxWidgets_LIBRARIES variable.
  # Also, Sets output variable wxWidgets_FOUND to FALSE if it fails.
  #
  MACRO(WX_SET_LIBRARIES _LIBS _DBG)
    IF(WX_USE_REL_AND_DBG)
      DBG_MSG("looking for ${${_LIBS}}")
      FOREACH(LIB ${${_LIBS}})
        DBG_MSG("Finding ${LIB} and ${LIB}d")
        DBG_MSG("WX_${LIB}  : ${WX_${LIB}}")
        DBG_MSG("WX_${LIB}d : ${WX_${LIB}d}")
        IF(WX_${LIB} AND WX_${LIB}d)
          DBG_MSG("Found ${LIB} and ${LIB}d")
          SET(wxWidgets_LIBRARIES ${wxWidgets_LIBRARIES}
            debug     ${WX_${LIB}d}
            optimized ${WX_${LIB}}
            )
        ELSE(WX_${LIB} AND WX_${LIB}d)
          DBG_MSG("- not found due to missing WX_${LIB}=${WX_${LIB}} or WX_${LIB}d=${WX_${LIB}d}")
          SET(wxWidgets_FOUND FALSE)
        ENDIF(WX_${LIB} AND WX_${LIB}d)
      ENDFOREACH(LIB)
    ELSE(WX_USE_REL_AND_DBG)
      FOREACH(LIB ${${_LIBS}})
        DBG_MSG("Finding ${LIB}${_DBG}")
        DBG_MSG("WX_${LIB}${_DBG} : ${WX_${LIB}${_DBG}}")
        IF(WX_${LIB}${_DBG})
          DBG_MSG("Found ${LIB}${_DBG}")
          SET(wxWidgets_LIBRARIES ${wxWidgets_LIBRARIES}
            ${WX_${LIB}${_DBG}}
            )
        ELSE(WX_${LIB}${_DBG})
          DBG_MSG("- not found due to missing WX_${LIB}${_DBG}=${WX_${LIB}${_DBG}}")
          SET(wxWidgets_FOUND FALSE)
        ENDIF(WX_${LIB}${_DBG})
      ENDFOREACH(LIB)
    ENDIF(WX_USE_REL_AND_DBG)

    FOREACH(LIB ${${_LIBS}})
      DBG_MSG("required: ${LIB}")
      IF(LIB STREQUAL "gl")
        DBG_MSG("gl required: ${LIB}")
        SET(wxWidgets_LIBRARIES ${wxWidgets_LIBRARIES}
          opengl32
          glu32
          )
      ENDIF(LIB STREQUAL "gl")
    ENDFOREACH(LIB ${${_LIBS}})

    SET(wxWidgets_LIBRARIES ${wxWidgets_LIBRARIES}
      winmm
      comctl32
      rpcrt4
      wsock32
      )
  ENDMACRO(WX_SET_LIBRARIES)

  #---------------------------------------------------------------------
  # WIN32: Start actual work.
  #---------------------------------------------------------------------
  #
  # Look for an installation tree.
  #
  FIND_PATH(wxWidgets_ROOT_DIR 
    NAMES include/wx/wx.h
    PATHS
    $ENV{wxWidgets_ROOT_DIR}
    $ENV{WXWIN}
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\wxWidgets_is1;Inno Setup: App Path]"  # WX 2.6.x
    C:/
    D:/
    $ENV{ProgramFiles}
    PATH_SUFFIXES 
    wxWidgets-2.8.4
    wxWidgets-2.8.3
    wxWidgets-2.8.2
    wxWidgets-2.8.1
    wxWidgets-2.8.0
    wxWidgets-2.7.4
    wxWidgets-2.7.3
    wxWidgets-2.7.2
    wxWidgest-2.7.1
    wxWidgets-2.7.0
    wxWidgets-2.7.0-1
    wxWidgets-2.6.3
    wxWidgets-2.6.2
    wxWidgets-2.6.1
    wxWidgets-2.5.4
    wxWidgets-2.5.3
    wxWidgets-2.5.2
    wxWidgets-2.5.1
    wxWidgets
    DOC "wxWidgets base/installation directory?"
    )

  # If wxWidgets_ROOT_DIR changed, clear all libraries and lib dir.
  IF(NOT WX_ROOT_DIR STREQUAL wxWidgets_ROOT_DIR)
    SET(WX_ROOT_DIR ${wxWidgets_ROOT_DIR} CACHE INTERNAL "wxWidgets_ROOT_DIR")
    #  WX_CLEAR_ALL_DBG_LIBS()
    #  WX_CLEAR_ALL_REL_LIBS()
    SET(wxWidgets_LIB_DIR "wxWidgets_LIB_DIR-NOTFOUND" CACHE PATH "Cleared." FORCE)
  ENDIF(NOT WX_ROOT_DIR STREQUAL wxWidgets_ROOT_DIR)

  IF(WX_ROOT_DIR)

    # select one default tree inside the already determined wx tree
    # prefer static/shared order usually consistent with build settings
    IF(BUILD_SHARED_LIBS)
      FIND_PATH(wxWidgets_LIB_DIR
        NAMES wxpng.lib wxpngd.lib
        PATHS
        ${WX_ROOT_DIR}/lib/vc_dll   # prefer shared
        ${WX_ROOT_DIR}/lib/vc_lib
        DOC "Path to wxWidgets libraries?"
        NO_DEFAULT_PATH
        )
    ELSE(BUILD_SHARED_LIBS)
      FIND_PATH(wxWidgets_LIB_DIR
        NAMES wxpng.lib wxpngd.lib
        PATHS
        ${WX_ROOT_DIR}/lib/vc_lib   # prefer static
        ${WX_ROOT_DIR}/lib/vc_dll
        DOC "Path to wxWidgets libraries?"
        NO_DEFAULT_PATH
        )
    ENDIF(BUILD_SHARED_LIBS)

    # If wxWidgets_LIB_DIR changed, clear all libraries.
    IF(NOT WX_LIB_DIR STREQUAL wxWidgets_LIB_DIR)
      SET(WX_LIB_DIR ${wxWidgets_LIB_DIR} CACHE INTERNAL "wxWidgets_LIB_DIR")
      WX_CLEAR_ALL_DBG_LIBS()
      WX_CLEAR_ALL_REL_LIBS()
    ENDIF(NOT WX_LIB_DIR STREQUAL wxWidgets_LIB_DIR)

    IF(WX_LIB_DIR)
      SET(wxWidgets_FOUND TRUE)

      IF(WX_LIB_DIR MATCHES ".*[dD][lL][lL].*")
        DBG_MSG("detected SHARED/DLL tree WX_LIB_DIR=${WX_LIB_DIR}")
        # add define for correct dllimport to link against WX DLL
        SET(wxWidgets_DEFINITIONS "-DWXUSINGDLL")
      ENDIF(WX_LIB_DIR MATCHES ".*[dD][lL][lL].*")

      #---------------------------------------------------------------------
      # WIN32: ???
      #---------------------------------------------------------------------
      # Search for possible configuration type availabilities
      # ***** SET(WX_LAST_CFG "")
      FOREACH(CFG mswunivud mswunivd mswud mswd mswunivu mswuniv mswu msw)
        SET(WX_${CFG}_FOUND FALSE)
        IF(EXISTS ${WX_LIB_DIR}/${CFG})
          SET(WX_CONFIGURATION_LIST ${WX_CONFIGURATION_LIST} ${CFG})
          SET(WX_${CFG}_FOUND TRUE)
          SET(WX_CONFIGURATION ${CFG})
        ENDIF(EXISTS ${WX_LIB_DIR}/${CFG})
      ENDFOREACH(CFG)

      # ***** SET(WX_USE_REL_AND_DBG FALSE)
      IF(WX_CONFIGURATION)
        # if selected configuration wasn't found, force the default one
        # else, use it but still force a refresh for the list in doc string
        IF(NOT WX_${wxWidgets_CONFIGURATION}_FOUND)
          SET(wxWidgets_CONFIGURATION ${WX_CONFIGURATION} CACHE STRING
            "Set wxWidgets configuration (${WX_CONFIGURATION_LIST})" FORCE)
        ELSE(NOT WX_${wxWidgets_CONFIGURATION}_FOUND)
          SET(wxWidgets_CONFIGURATION ${wxWidgets_CONFIGURATION} CACHE STRING
            "Set wxWidgets configuration (${WX_CONFIGURATION_LIST})" FORCE)
        ENDIF(NOT WX_${wxWidgets_CONFIGURATION}_FOUND)

        # if release config was selected, and both release/debug exist
        IF(WX_${wxWidgets_CONFIGURATION}d_FOUND)
          OPTION(wxWidgets_USE_REL_AND_DBG
            "Use release and debug configurations?" TRUE)
          SET(WX_USE_REL_AND_DBG ${wxWidgets_USE_REL_AND_DBG})
        ELSE(WX_${wxWidgets_CONFIGURATION}d_FOUND)
          # if the option exists, force it to false
          IF(wxWidgets_USE_REL_AND_DBG)
            SET(wxWidgets_USE_REL_AND_DBG FALSE CACHE BOOL
              "No ${wxWidgets_CONFIGURATION}d found." FORCE)
          ENDIF(wxWidgets_USE_REL_AND_DBG)
          SET(WX_USE_REL_AND_DBG FALSE)
        ENDIF(WX_${wxWidgets_CONFIGURATION}d_FOUND)

        # Get configuration parameters from the name.
        WX_GET_NAME_COMPONENTS(${wxWidgets_CONFIGURATION} UNV UCD DBG)

        # Set wxWidgets main include directory.
        IF(EXISTS ${WX_ROOT_DIR}/include/wx/wx.h)
          SET(wxWidgets_INCLUDE_DIRS ${WX_ROOT_DIR}/include)
        ELSE(EXISTS ${WX_ROOT_DIR}/include/wx/wx.h)
          DBG_MSG("WXWIDGET_FOUND FALSE because WX_ROOT_DIR=${WX_ROOT_DIR} has no ${WX_ROOT_DIR}/include/wx/wx.h")
          SET(wxWidgets_FOUND FALSE)
        ENDIF(EXISTS ${WX_ROOT_DIR}/include/wx/wx.h)

        # Set wxWidgets lib setup include directory.
        IF(EXISTS ${WX_LIB_DIR}/${wxWidgets_CONFIGURATION}/wx/setup.h)
          SET(wxWidgets_INCLUDE_DIRS ${wxWidgets_INCLUDE_DIRS}
            ${WX_LIB_DIR}/${wxWidgets_CONFIGURATION})
        ELSE(EXISTS ${WX_LIB_DIR}/${wxWidgets_CONFIGURATION}/wx/setup.h)
          DBG_MSG("WXWIDGET_FOUND FALSE because  ${WX_LIB_DIR}/${wxWidgets_CONFIGURATION}/wx/setup.h does not exists.")
          SET(wxWidgets_FOUND FALSE)
        ENDIF(EXISTS ${WX_LIB_DIR}/${wxWidgets_CONFIGURATION}/wx/setup.h)
        #FIND_PATH(WX_SETUP_INCLUDE_DIR wx/setup.h
        #          ${WX_LIB_DIR}/${wxWidgets_CONFIGURATION})
        #MARK_AS_ADVANCED(WX_SETUP_INCLUDE_DIR)

        # Find wxWidgets libraries.
        WX_FIND_LIBS("${UNV}" "${UCD}" "${DBG}")
        IF(WX_USE_REL_AND_DBG)
          WX_FIND_LIBS("${UNV}" "${UCD}" "d")
        ENDIF(WX_USE_REL_AND_DBG)

        # we support adding components by _USE_LIBS or REQUIRED _COMPONENTS
        IF   (wxWidgets_FIND_COMPONENTS)
          LIST(APPEND wxWidgets_USE_LIBS ${wxWidgets_FIND_COMPONENTS})
        ENDIF(wxWidgets_FIND_COMPONENTS)



        # Libraries we are interested in.
        IF(NOT wxWidgets_USE_LIBS)
          # Default minimal use setting (i.e., link to only core,base).
          SET(wxWidgets_USE_LIBS ${wxWidgets_STD_LIBRARIES} )
        ENDIF(NOT wxWidgets_USE_LIBS)

        IF (wxWidgets_USE_LIBS MATCHES std)
          # replace std by the list of STD libs
          LIST(APPEND wxWidgets_USE_LIBS ${wxWidgets_STD_LIBRARIES} )
          LIST(REMOVE_ITEM wxWidgets_USE_LIBS std)
          # TODO: check that "mono"  and base,core aren't added together
        ENDIF (wxWidgets_USE_LIBS MATCHES std)

        # Always add the common required libs.
        LIST(APPEND wxWidgets_USE_LIBS ${wxWidgets_COMMON_LIBRARIES} )

        # Settings for requested libs (i.e., include dir, libraries, etc.).
        WX_SET_LIBRARIES(wxWidgets_USE_LIBS "${DBG}")

      ENDIF(WX_CONFIGURATION)
    ENDIF(WX_LIB_DIR)
  ENDIF(WX_ROOT_DIR)


  #=====================================================================
  #=====================================================================
ELSE(WIN32_STYLE_FIND)
  IF(UNIX_STYLE_FIND)

    FIND_PROGRAM(wxWidgets_CONFIG_EXECUTABLE wx-config)
    IF(wxWidgets_CONFIG_EXECUTABLE)
      SET(wxWidgets_FOUND TRUE)

      # run the wx-config program to get cxxflags
      EXEC_PROGRAM(sh
        ARGS "${wxWidgets_CONFIG_EXECUTABLE} --cxxflags"
        OUTPUT_VARIABLE wxWidgets_CXX_FLAGS
        RETURN_VALUE RET)
      IF(RET EQUAL 0)
        # parse definitions from cxxflags
        STRING(REGEX MATCHALL "-D.*[^ ;]+" wxWidgets_DEFINITIONS  ${wxWidgets_CXX_FLAGS})
        DBG_MSG("\nwxWidgets_DEFINITIONS=${wxWidgets_DEFINITIONS}")
        # drop -D* from CXXFLAGS
        STRING(REGEX REPLACE "-D[^ ;]*" ""  wxWidgets_CXX_FLAGS  ${wxWidgets_CXX_FLAGS})

        # parse incdirs from cxxflags, drop -I prefix
        STRING(REGEX MATCHALL "-I.*[^ ;]+" wxWidgets_INCLUDE_DIRS  ${wxWidgets_CXX_FLAGS})
        STRING(REGEX REPLACE "-I" ""  wxWidgets_INCLUDE_DIRS "${wxWidgets_INCLUDE_DIRS}")
        # convert space to semicolons for list
        STRING(REGEX REPLACE " " ";" wxWidgets_INCLUDE_DIRS "${wxWidgets_INCLUDE_DIRS}")

      ELSE(RET EQUAL 0)
        DBG_MSG("${wxWidgets_CONFIG_EXECUTABLE} --cxxflags FAILED with RET=${RET}")
        SET(wxWidgets_FOUND FALSE)
      ENDIF(RET EQUAL 0)


      # run the wx-config program to get the libs
      # - NOTE: wx-config doesn't verify that the libs requested exist
      #         it just produces the names. Maybe a TRY_COMPILE would
      #         be useful here...
      #STRING(REPLACE ";" "," wxWidgets_USE_LIBS "${wxWidgets_USE_LIBS}")
      STRING(REGEX REPLACE ";" "," wxWidgets_USE_LIBS "${wxWidgets_USE_LIBS}")
      EXEC_PROGRAM(sh
        ARGS "${wxWidgets_CONFIG_EXECUTABLE} --libs ${wxWidgets_USE_LIBS}"
        OUTPUT_VARIABLE wxWidgets_LIBRARIES
        RETURN_VALUE RET)
      IF(RET EQUAL 0)
        STRING(REGEX REPLACE " " ";" wxWidgets_LIBRARIES "${wxWidgets_LIBRARIES}")
        STRING(REGEX REPLACE "-framework;" "-framework "
          wxWidgets_LIBRARIES
          "${wxWidgets_LIBRARIES}")

        # extract linkdirs (-L) for rpath (i.e., LINK_DIRECTORIES)
        STRING(REGEX MATCHALL "-L[^ ;]+"
          wxWidgets_LIBRARY_DIRS
          "${wxWidgets_LIBRARIES}")
        STRING(REGEX REPLACE "-L" ""
          wxWidgets_LIBRARY_DIRS "${wxWidgets_LIBRARY_DIRS}")

        # convert space to semicolons for list
        STRING(REGEX REPLACE " " ";" wxWidgets_LIBRARY_DIRS "${wxWidgets_LIBRARY_DIRS}")
      ELSE(RET EQUAL 0)
        SET(wxWidgets_FOUND FALSE)
      ENDIF(RET EQUAL 0)
    ENDIF(wxWidgets_CONFIG_EXECUTABLE)

  ELSE(UNIX_STYLE_FIND)
    IF(NOT wxWidgets_FIND_QUIETLY)
      MESSAGE(STATUS "${CMAKE_CURRENT_LIST_FILE}(${CMAKE_CURRENT_LIST_LINE}): \n"
        "  Platform unknown/unsupported. It's neither WIN32 nor UNIX style find.")
    ENDIF(NOT wxWidgets_FIND_QUIETLY)
  ENDIF(UNIX_STYLE_FIND)
ENDIF(WIN32_STYLE_FIND)


# add convenience use file
IF  (wxWidgets_FOUND)
  # get dir of this file which may reside in
  # - CMAKE_MAKE_ROOT/Modules  on CMake installation
  # - CMAKE_MODULE_PATH if user prefers his own specialized version
  GET_FILENAME_COMPONENT(wxWidgets_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
  SET(wxWidgets_USE_FILE "${wxWidgets_CURRENT_LIST_DIR}/UsewxWidgets.cmake")
  # check
  IF   (NOT EXISTS ${wxWidgets_USE_FILE})
    IF   (NOT  wxWidgets_FIND_QUIETLY)
      MESSAGE(SEND_ERROR "Your Find/Use wxWidgets installation is wrong. wxWidgets_USE_FILE=${wxWidgets_USE_FILE} not found.")
    ENDIF(NOT  wxWidgets_FIND_QUIETLY)
  ENDIF(NOT EXISTS ${wxWidgets_USE_FILE})
ENDIF(wxWidgets_FOUND)

DBG_MSG("wxWidgets_FOUND           : ${wxWidgets_FOUND}")
DBG_MSG("wxWidgets_INCLUDE_DIRS    : ${wxWidgets_INCLUDE_DIRS}")
DBG_MSG("wxWidgets_LIBRARY_DIRS    : ${wxWidgets_LIBRARY_DIRS}")
DBG_MSG("wxWidgets_CXX_FLAGS       : ${wxWidgets_CXX_FLAGS}")
DBG_MSG("wxWidgets_USE_FILE        : ${wxWidgets_USE_FILE}")
DBG_MSG("wxWidgets_FIND_COMPONENTS : ${wxWidgets_FIND_COMPONENTS}")
DBG_MSG("wxWidgets_USE_LIBS        : ${wxWidgets_USE_LIBS}")

#=====================================================================
#=====================================================================
IF(NOT wxWidgets_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT wxWidgets_FIND_QUIETLY)
    IF(wxWidgets_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR
        "wxWidgets required, please specify it's location.")
    ELSE(wxWidgets_FIND_REQUIRED)
      MESSAGE(STATUS "ERROR: wxWidgets was not found.")
    ENDIF(wxWidgets_FIND_REQUIRED)
  ENDIF(NOT wxWidgets_FIND_QUIETLY)
ENDIF(NOT wxWidgets_FOUND)
