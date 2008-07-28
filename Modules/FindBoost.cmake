# - Try to find Boost include dirs and libraries
# Usage of this module as follows:
#
#     SET(Boost_USE_STATIC_LIBS ON)
#     SET(Boost_USE_MULTITHREAD OFF)
#     FIND_PACKAGE( Boost 1.34.1 COMPONENTS date_time filesystem iostreams ... )
#
# The Boost_ADDITIONAL_VERSIONS variable can be used to specify a list of
# boost version numbers that should be taken into account when searching
# for the libraries. Unfortunately boost puts the version number into the
# actual filename for the libraries, so this might be needed in the future
# when new Boost versions are released.
#
# Currently this module searches for the following version numbers:
# 1.33, 1.33.0, 1.33.1, 1.34, 1.34.0, 1.34.1, 1.35, 1.35.0, 1.35.1, 1.36, 
# 1.36.0, 1.36.1
#
# The components list needs to be the actual names of boost libraries, that is
# the part of the actual library files that differ on different libraries. So
# its "date_time" for "libboost_date_time...". Anything else will result in
# errors
#
# You can provide a minimum version number that should be used. If you provide this 
# version number and specify the REQUIRED attribute, this module will fail if it
# can't find the specified or a later version. If you specify a version number this is
# automatically put into the considered list of version numbers and thus doesn't need
# to be specified in the Boost_ADDITIONAL_VERSIONS variable
#
# Variables used by this module, they can change the default behaviour and need to be set
# before calling find_package:
#  Boost_USE_MULTITHREAD         Can be set to OFF to use the non-multithreaded
#                                boost libraries. Defaults to ON.
#  Boost_USE_STATIC_LIBS         Can be set to ON to force the use of the static
#                                boost libraries. Defaults to OFF.
#  Boost_ADDITIONAL_VERSIONS     A list of version numbers to use for searching
#                                the boost include directory. The default list
#                                of version numbers is:
#                                1.33, 1.33.0, 1.33.1, 1.34, 1.34.0, 1.34.1, 
#                                1.35, 1.35.0, 1.35.1, 1.36, 1.36.0, 1.36.1
#                                If you want to look for an older or newer
#                                version set this variable to a list of
#                                strings, where each string contains a number, i.e.
#                                SET(Boost_ADDITIONAL_VERSIONS "0.99.0" "1.35.0")
#  BOOST_ROOT or BOOSTROOT       Preferred installation prefix for searching for Boost,
#                                set this if the module has problems finding the proper Boost installation
#  BOOST_INCLUDEDIR              Set this to the include directory of Boost, if the
#                                module has problems finding the proper Boost installation
#  BOOST_LIBRARYDIR              Set this to the lib directory of Boost, if the
#                                module has problems finding the proper Boost installation
#
#  The last three variables are available also as environment variables
#
#
# Variables defined by this module:
#
#  Boost_FOUND                          System has Boost, this means the include dir was found,
#                                       as well as all the libraries specified in the COMPONENTS list
#  Boost_INCLUDE_DIRS                   Boost include directories, not cached
#  Boost_INCLUDE_DIR                    This is almost the same as above, but this one is cached and may be
#                                       modified by advanced users
#  Boost_LIBRARIES                      Link these to use the Boost libraries that you specified, not cached
#  Boost_LIBRARY_DIRS                   The path to where the Boost library files are.
#  Boost_VERSION                        The version number of the boost libraries that have been found,
#                                       same as in version.hpp from Boost
#  Boost_LIB_VERSION                    The version number in filename form as its appended to the library filenames
#  Boost_MAJOR_VERSION                  major version number of boost
#  Boost_MINOR_VERSION                  minor version number of boost
#  Boost_SUBMINOR_VERSION               subminor version number of boost
#  Boost_LIB_DIAGNOSTIC_DEFINITIONS     Only set on windows. Can be used with add_definitions 
#                                       to print diagnostic information about the automatic 
#                                       linking done on windows.

# For each component you list the following variables are set.
# ATTENTION: The component names need to be in lower case, just as the boost
# library names however the cmake variables use upper case for the component
# part. So you'd get Boost_SERIALIZATION_FOUND for example.
#
#  Boost_${COMPONENT}_FOUND             True IF the Boost library "component" was found.
#  Boost_${COMPONENT}_LIBRARY           The absolute path of the Boost library "component".
#  Boost_${COMPONENT}_LIBRARY_DEBUG     The absolute path of the debug version of the
#                                       Boost library "component".
#  Boost_${COMPONENT}_LIBRARY_RELEASE   The absolute path of the release version of the
#                                       Boost library "component"
#
#  Copyright (c) 2006-2008 Andreas Schneider <mail@cynapses.org>
#  Copyright (c) 2007      Wengo
#  Copyright (c) 2007      Mike Jackson
#  Copyright (c) 2008      Andreas Pakulat <apaku@gmx.de>
#
#  Redistribution AND use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
OPTION(Boost_USE_MULTITHREADED 
  "Use the multithreaded versions of the Boost libraries" ON)

if (Boost_FIND_VERSION_EXACT)
  if (Boost_FIND_VERSION_PATCH)
    set( _boost_TEST_VERSIONS 
      "${Boost_FIND_VERSION_MAJOR}.${Boost_FIND_VERSION_MINOR}.${Boost_FIND_VERSION_PATCH}")
  else (Boost_FIND_VERSION_PATCH)
    set( _boost_TEST_VERSIONS 
      "${Boost_FIND_VERSION_MAJOR}.${Boost_FIND_VERSION_MINOR}.0"
      "${Boost_FIND_VERSION_MAJOR}.${Boost_FIND_VERSION_MINOR}")
  endif (Boost_FIND_VERSION_PATCH)
else (Boost_FIND_VERSION_EXACT)
  set( _boost_TEST_VERSIONS ${Boost_ADDITIONAL_VERSIONS} 
    "1.36.1" "1.36.0" "1.36" "1.35.1" "1.35.0" "1.35" "1.34.1" "1.34.0" 
    "1.34" "1.33.1" "1.33.0" "1.33" )
endif (Boost_FIND_VERSION_EXACT)

# The reason that we failed to find Boost. This will be set to a
# user-friendly message when we fail to find some necessary piece of
# Boost.
set(Boost_ERROR_REASON)

############################################
#
# Check the existence of the libraries.
#
############################################
# This macro was taken directly from the FindQt4.cmake file that is included
# with the CMake distribution. This is NOT my work. All work was done by the
# original authors of the FindQt4.cmake file. Only minor modifications were
# made to remove references to Qt and make this file more generally applicable
#########################################################################

MACRO (_Boost_ADJUST_LIB_VARS basename)
  IF (Boost_INCLUDE_DIR )
    IF (Boost_${basename}_LIBRARY_DEBUG AND Boost_${basename}_LIBRARY_RELEASE)
      # if the generator supports configuration types then set
      # optimized and debug libraries, or if the CMAKE_BUILD_TYPE has a value
      IF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
        SET(Boost_${basename}_LIBRARY optimized ${Boost_${basename}_LIBRARY_RELEASE} debug ${Boost_${basename}_LIBRARY_DEBUG})
      ELSE(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
        # if there are no configuration types and CMAKE_BUILD_TYPE has no value
        # then just use the release libraries
        SET(Boost_${basename}_LIBRARY ${Boost_${basename}_LIBRARY_RELEASE} )
      ENDIF(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
      SET(Boost_${basename}_LIBRARIES optimized ${Boost_${basename}_LIBRARY_RELEASE} debug ${Boost_${basename}_LIBRARY_DEBUG})
    ENDIF (Boost_${basename}_LIBRARY_DEBUG AND Boost_${basename}_LIBRARY_RELEASE)

    # if only the release version was found, set the debug variable also to the release version
    IF (Boost_${basename}_LIBRARY_RELEASE AND NOT Boost_${basename}_LIBRARY_DEBUG)
      SET(Boost_${basename}_LIBRARY_DEBUG ${Boost_${basename}_LIBRARY_RELEASE})
      SET(Boost_${basename}_LIBRARY       ${Boost_${basename}_LIBRARY_RELEASE})
      SET(Boost_${basename}_LIBRARIES     ${Boost_${basename}_LIBRARY_RELEASE})
    ENDIF (Boost_${basename}_LIBRARY_RELEASE AND NOT Boost_${basename}_LIBRARY_DEBUG)

    # if only the debug version was found, set the release variable also to the debug version
    IF (Boost_${basename}_LIBRARY_DEBUG AND NOT Boost_${basename}_LIBRARY_RELEASE)
      SET(Boost_${basename}_LIBRARY_RELEASE ${Boost_${basename}_LIBRARY_DEBUG})
      SET(Boost_${basename}_LIBRARY         ${Boost_${basename}_LIBRARY_DEBUG})
      SET(Boost_${basename}_LIBRARIES       ${Boost_${basename}_LIBRARY_DEBUG})
    ENDIF (Boost_${basename}_LIBRARY_DEBUG AND NOT Boost_${basename}_LIBRARY_RELEASE)
    
    IF (Boost_${basename}_LIBRARY)
      SET(Boost_${basename}_LIBRARY ${Boost_${basename}_LIBRARY} CACHE FILEPATH "The Boost ${basename} library")
      GET_FILENAME_COMPONENT(Boost_LIBRARY_DIRS "${Boost_${basename}_LIBRARY}" PATH)
      SET(Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIRS} CACHE FILEPATH "Boost library directory")
      SET(Boost_${basename}_FOUND ON CACHE INTERNAL "Whether the Boost ${basename} library found")
    ENDIF (Boost_${basename}_LIBRARY)

  ENDIF (Boost_INCLUDE_DIR )
  # Make variables changeble to the advanced user
  MARK_AS_ADVANCED(
      Boost_${basename}_LIBRARY
      Boost_${basename}_LIBRARY_RELEASE
      Boost_${basename}_LIBRARY_DEBUG
  )
ENDMACRO (_Boost_ADJUST_LIB_VARS)

#-------------------------------------------------------------------------------


SET( _boost_IN_CACHE TRUE)
IF(Boost_INCLUDE_DIR)
  FOREACH(COMPONENT ${Boost_FIND_COMPONENTS})
    STRING(TOUPPER ${COMPONENT} COMPONENT)
    IF(NOT Boost_${COMPONENT}_FOUND)
      SET( _boost_IN_CACHE FALSE)
    ENDIF(NOT Boost_${COMPONENT}_FOUND)
  ENDFOREACH(COMPONENT)
ELSE(Boost_INCLUDE_DIR)
  SET( _boost_IN_CACHE FALSE)
ENDIF(Boost_INCLUDE_DIR)

IF (_boost_IN_CACHE)
  # in cache already
  SET(Boost_FOUND TRUE)
  FOREACH(COMPONENT ${Boost_FIND_COMPONENTS})
    STRING(TOUPPER ${COMPONENT} COMPONENT)
    _Boost_ADJUST_LIB_VARS( ${COMPONENT} )
    SET(Boost_LIBRARIES ${Boost_LIBRARIES} ${Boost_${COMPONENT}_LIBRARY})
  ENDFOREACH(COMPONENT)
  SET(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIR})
  IF(Boost_VERSION AND NOT "${Boost_VERSION}" STREQUAL "0")
    MATH(EXPR Boost_MAJOR_VERSION "${Boost_VERSION} / 100000")
    MATH(EXPR Boost_MINOR_VERSION "${Boost_VERSION} / 100 % 1000")
    MATH(EXPR Boost_SUBMINOR_VERSION "${Boost_VERSION} % 100")
  ENDIF(Boost_VERSION AND NOT "${Boost_VERSION}" STREQUAL "0")
ELSE (_boost_IN_CACHE)
  # Need to search for boost

  IF(WIN32)
    # In windows, automatic linking is performed, so you do not have
    # to specify the libraries.  If you are linking to a dynamic
    # runtime, then you can choose to link to either a static or a
    # dynamic Boost library, the default is to do a static link.  You
    # can alter this for a specific library "whatever" by defining
    # BOOST_WHATEVER_DYN_LINK to force Boost library "whatever" to be
    # linked dynamically.  Alternatively you can force all Boost
    # libraries to dynamic link by defining BOOST_ALL_DYN_LINK.
  
    # This feature can be disabled for Boost library "whatever" by
    # defining BOOST_WHATEVER_NO_LIB, or for all of Boost by defining
    # BOOST_ALL_NO_LIB.
  
    # If you want to observe which libraries are being linked against
    # then defining BOOST_LIB_DIAGNOSTIC will cause the auto-linking
    # code to emit a #pragma message each time a library is selected
    # for linking.
    SET(Boost_LIB_DIAGNOSTIC_DEFINITIONS 
      "-DBOOST_LIB_DIAGNOSTIC" CACHE STRING "Boost diagnostic define")
  ENDIF(WIN32)

  SET(_boost_INCLUDE_SEARCH_DIRS
    C:/boost/include
    "C:/boost"
    "$ENV{ProgramFiles}/boost/boost_${Boost_FIND_VERSION_MAJOR}_${Boost_FIND_VERSION_MINOR}_${Boost_FIND_VERSION_PATCH}"
    "$ENV{ProgramFiles}/Boost"
    /sw/local/include
  )

  SET(_boost_LIBRARIES_SEARCH_DIRS
    C:/boost/lib
    "C:/boost"
    "$ENV{ProgramFiles}/boost/boost_${Boost_FIND_VERSION_MAJOR}_${Boost_FIND_VERSION_MINOR}_${Boost_FIND_VERSION_PATCH}/lib"
    "$ENV{ProgramFiles}/Boost"
    /sw/local/lib
  )

  # If BOOST_ROOT was defined in the environment, use it.
  if (NOT BOOST_ROOT AND NOT $ENV{BOOST_ROOT} STREQUAL "")
    set(BOOST_ROOT $ENV{BOOST_ROOT})
  endif(NOT BOOST_ROOT AND NOT $ENV{BOOST_ROOT} STREQUAL "")

  # If BOOSTROOT was defined in the environment, use it.
  if (NOT BOOST_ROOT AND NOT $ENV{BOOSTROOT} STREQUAL "")
    set(BOOST_ROOT $ENV{BOOSTROOT})
  endif(NOT BOOST_ROOT AND NOT $ENV{BOOSTROOT} STREQUAL "")

  # If BOOST_INCLUDEDIR was defined in the environment, use it.
  IF( NOT $ENV{BOOST_INCLUDEDIR} STREQUAL "" )
    set(BOOST_INCLUDEDIR $ENV{BOOST_INCLUDEDIR})
  ENDIF( NOT $ENV{BOOST_INCLUDEDIR} STREQUAL "" )

  # If BOOST_LIBRARYDIR was defined in the environment, use it.
  IF( NOT $ENV{BOOST_LIBRARYDIR} STREQUAL "" )
    set(BOOST_LIBRARYDIR $ENV{BOOST_LIBRARYDIR})
  ENDIF( NOT $ENV{BOOST_LIBRARYDIR} STREQUAL "" )

  IF( BOOST_ROOT )
    file(TO_CMAKE_PATH ${BOOST_ROOT} BOOST_ROOT)
    SET(_boost_INCLUDE_SEARCH_DIRS 
      ${BOOST_ROOT}/include 
      ${BOOST_ROOT}
      ${_boost_INCLUDE_SEARCH_DIRS})
    SET(_boost_LIBRARIES_SEARCH_DIRS 
      ${BOOST_ROOT}/lib 
      ${BOOST_ROOT}/stage/lib 
      ${_boost_LIBRARIES_SEARCH_DIRS})
  ENDIF( BOOST_ROOT )

  IF( BOOST_INCLUDEDIR )
    file(TO_CMAKE_PATH ${BOOST_INCLUDEDIR} BOOST_INCLUDEDIR)
    SET(_boost_INCLUDE_SEARCH_DIRS 
      ${BOOST_INCLUDEDIR} ${_boost_INCLUDE_SEARCH_DIRS})
  ENDIF( BOOST_INCLUDEDIR )

  IF( BOOST_LIBRARYDIR )
    file(TO_CMAKE_PATH ${BOOST_LIBRARYDIR} BOOST_LIBRARYDIR)
    SET(_boost_LIBRARIES_SEARCH_DIRS 
      ${BOOST_LIBRARYDIR} ${_boost_LIBRARIES_SEARCH_DIRS})
  ENDIF( BOOST_LIBRARYDIR )

  # Try to find Boost by stepping backwards through the Boost versions
  # we know about.
  FOREACH(_boost_VER ${_boost_TEST_VERSIONS})
    IF( NOT Boost_INCLUDE_DIR )
      # Add in a path suffix, based on the required version, ideally
      # we could read this from version.hpp, but for that to work we'd
      # need to know the include dir already
      if (WIN32 AND NOT CYGWIN)
        set(_boost_PATH_SUFFIX boost_${_boost_VER})
      else (WIN32 AND NOT CYGWIN)
        set(_boost_PATH_SUFFIX boost-${_boost_VER})
      endif (WIN32 AND NOT CYGWIN)

      IF(_boost_PATH_SUFFIX MATCHES "[0-9]+\\.[0-9]+\\.[0-9]+")
          STRING(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\1_\\2_\\3" 
            _boost_PATH_SUFFIX ${_boost_PATH_SUFFIX})
      ELSEIF(_boost_PATH_SUFFIX MATCHES "[0-9]+\\.[0-9]+")
          STRING(REGEX REPLACE "([0-9]+)\\.([0-9]+)" "\\1_\\2" 
            _boost_PATH_SUFFIX ${_boost_PATH_SUFFIX})
      ENDIF(_boost_PATH_SUFFIX MATCHES "[0-9]+\\.[0-9]+\\.[0-9]+")

      FIND_PATH(Boost_INCLUDE_DIR
          NAMES         boost/config.hpp
          HINTS         ${_boost_INCLUDE_SEARCH_DIRS}
          PATH_SUFFIXES ${_boost_PATH_SUFFIX}
      )

    ENDIF( NOT Boost_INCLUDE_DIR )
  ENDFOREACH(_boost_VER)

  IF(Boost_INCLUDE_DIR)
    # Extract Boost_VERSION and Boost_LIB_VERSION from version.hpp
    # Read the whole file:
    #
    SET(BOOST_VERSION 0)
    SET(BOOST_LIB_VERSION "")
    FILE(READ "${Boost_INCLUDE_DIR}/boost/version.hpp" _boost_VERSION_HPP_CONTENTS)
  
    STRING(REGEX REPLACE ".*#define BOOST_VERSION ([0-9]+).*" "\\1" Boost_VERSION "${_boost_VERSION_HPP_CONTENTS}")
    STRING(REGEX REPLACE ".*#define BOOST_LIB_VERSION \"([0-9_]+)\".*" "\\1" Boost_LIB_VERSION "${_boost_VERSION_HPP_CONTENTS}")
  
    SET(Boost_LIB_VERSION ${Boost_LIB_VERSION} CACHE INTERNAL "The library version string for boost libraries")
    SET(Boost_VERSION ${Boost_VERSION} CACHE INTERNAL "The version number for boost libraries")
    
    IF(NOT "${Boost_VERSION}" STREQUAL "0")
      MATH(EXPR Boost_MAJOR_VERSION "${Boost_VERSION} / 100000")
      MATH(EXPR Boost_MINOR_VERSION "${Boost_VERSION} / 100 % 1000")
      MATH(EXPR Boost_SUBMINOR_VERSION "${Boost_VERSION} % 100")

      set(Boost_ERROR_REASON
          "${Boost_ERROR_REASON}Boost version: ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}\nBoost include path: ${Boost_INCLUDE_DIR}")
    ENDIF(NOT "${Boost_VERSION}" STREQUAL "0")
  ELSE(Boost_INCLUDE_DIR)
    set(Boost_ERROR_REASON
      "${Boost_ERROR_REASON}Unable to find the Boost header files. Please set BOOST_ROOT to the root directory containing Boost or BOOST_INCLUDEDIR to the directory containing Boost's headers.")
  ENDIF(Boost_INCLUDE_DIR)

  # Setting some more suffixes for the library
  SET (Boost_LIB_PREFIX "")
  IF ( WIN32 AND Boost_USE_STATIC_LIBS )
    SET (Boost_LIB_PREFIX "lib")
  ENDIF ( WIN32 AND Boost_USE_STATIC_LIBS )
  SET (_boost_COMPILER "-gcc")
  IF (MSVC90)
    SET (_boost_COMPILER "-vc90")
  ELSEIF (MSVC80)
    SET (_boost_COMPILER "-vc80")
  ELSEIF (MSVC71)
    SET (_boost_COMPILER "-vc71")
  ENDIF(MSVC90)
  IF (MINGW)
    EXEC_PROGRAM(${CMAKE_CXX_COMPILER}
      ARGS -dumpversion
      OUTPUT_VARIABLE _boost_COMPILER_VERSION
      )
    STRING(REGEX REPLACE "([0-9])\\.([0-9])\\.[0-9]" "\\1\\2"
      _boost_COMPILER_VERSION ${_boost_COMPILER_VERSION})
    SET (_boost_COMPILER "-mgw${_boost_COMPILER_VERSION}")
  ENDIF(MINGW)
  IF (UNIX)
    IF (NOT CMAKE_COMPILER_IS_GNUCC)
      # We assume that we have the Intel compiler.
      SET (_boost_COMPILER "-il")
    ELSE (NOT CMAKE_COMPILER_IS_GNUCC)
      # Determine which version of GCC we have.
      EXEC_PROGRAM(${CMAKE_CXX_COMPILER}
        ARGS -dumpversion
        OUTPUT_VARIABLE _boost_COMPILER_VERSION
        )
      STRING(REGEX REPLACE "([0-9])\\.([0-9])\\.[0-9]" "\\1\\2"
        _boost_COMPILER_VERSION ${_boost_COMPILER_VERSION})
      IF(APPLE)
        IF(Boost_MINOR_VERSION)
          IF(${Boost_MINOR_VERSION} GREATER 35)
            # In Boost 1.36.0 and newer, the mangled compiler name used
            # on Mac OS X/Darwin is "xgcc".
            SET(_boost_COMPILER "-xgcc${_boost_COMPILER_VERSION}")
          ELSE(${Boost_MINOR_VERSION} GREATER 35)
            # In Boost <= 1.35.0, there is no mangled compiler name for
            # the Mac OS X/Darwin version of GCC.
            SET(_boost_COMPILER "")
          ENDIF(${Boost_MINOR_VERSION} GREATER 35)
        ELSE(Boost_MINOR_VERSION)
          # We don't know the Boost version, so assume it's
          # pre-1.36.0.
          SET(_boost_COMPILER "")
        ENDIF(Boost_MINOR_VERSION)
      ELSE()
        SET (_boost_COMPILER "-gcc${_boost_COMPILER_VERSION}")
      ENDIF()
    ENDIF (NOT CMAKE_COMPILER_IS_GNUCC)
  ENDIF(UNIX)

  SET (_boost_MULTITHREADED "-mt")

  IF( NOT Boost_USE_MULTITHREADED )
    SET (_boost_MULTITHREADED "")
  ENDIF( NOT Boost_USE_MULTITHREADED )

  SET( _boost_STATIC_TAG "")
  IF (WIN32)
    IF(MSVC)
      SET (_boost_ABI_TAG "g")
    ENDIF(MSVC)
    IF( Boost_USE_STATIC_LIBS )
      SET( _boost_STATIC_TAG "-s")
    ENDIF( Boost_USE_STATIC_LIBS )
  ENDIF(WIN32)
  SET (_boost_ABI_TAG "${_boost_ABI_TAG}d")

  # ------------------------------------------------------------------------
  #  Begin finding boost libraries
  # ------------------------------------------------------------------------
  FOREACH(COMPONENT ${Boost_FIND_COMPONENTS})
    STRING(TOUPPER ${COMPONENT} UPPERCOMPONENT)
    SET( Boost_${UPPERCOMPONENT}_LIBRARY "Boost_${UPPERCOMPONENT}_LIBRARY-NOTFOUND" )
    SET( Boost_${UPPERCOMPONENT}_LIBRARY_RELEASE "Boost_${UPPERCOMPONENT}_LIBRARY_RELEASE-NOTFOUND" )
    SET( Boost_${UPPERCOMPONENT}_LIBRARY_DEBUG "Boost_${UPPERCOMPONENT}_LIBRARY_DEBUG-NOTFOUND")

    # Support preference of static libs by adjusting CMAKE_FIND_LIBRARY_SUFFIXES
    IF( Boost_USE_STATIC_LIBS )
      SET( _boost_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
      IF(WIN32)
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
      ELSE(WIN32)
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
      ENDIF(WIN32)
    ENDIF( Boost_USE_STATIC_LIBS )

    FIND_LIBRARY(Boost_${UPPERCOMPONENT}_LIBRARY_RELEASE
        NAMES  ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_COMPILER}${_boost_MULTITHREADED}-${Boost_LIB_VERSION}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_COMPILER}${_boost_MULTITHREADED}${_boost_STATIC_TAG}-${Boost_LIB_VERSION}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_MULTITHREADED}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_MULTITHREADED}${_boost_STATIC_TAG}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}
        HINTS  ${_boost_LIBRARIES_SEARCH_DIRS}
    )

    FIND_LIBRARY(Boost_${UPPERCOMPONENT}_LIBRARY_DEBUG
        NAMES  ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_COMPILER}${_boost_MULTITHREADED}-${_boost_ABI_TAG}-${Boost_LIB_VERSION}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_COMPILER}${_boost_MULTITHREADED}${_boost_STATIC_TAG}${_boost_ABI_TAG}-${Boost_LIB_VERSION}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_MULTITHREADED}-${_boost_ABI_TAG}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}${_boost_MULTITHREADED}${_boost_STATIC_TAG}${_boost_ABI_TAG}
               ${Boost_LIB_PREFIX}boost_${COMPONENT}-${_boost_ABI_TAG}
        HINTS  ${_boost_LIBRARIES_SEARCH_DIRS}
    )

    _Boost_ADJUST_LIB_VARS(${UPPERCOMPONENT})
    IF( Boost_USE_STATIC_LIBS )
      SET(CMAKE_FIND_LIBRARY_SUFFIXES ${_boost_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})
    ENDIF( Boost_USE_STATIC_LIBS )
  ENDFOREACH(COMPONENT)
  # ------------------------------------------------------------------------
  #  End finding boost libraries
  # ------------------------------------------------------------------------

  SET(Boost_INCLUDE_DIRS
    ${Boost_INCLUDE_DIR}
  )

  SET(Boost_FOUND FALSE)
  IF(Boost_INCLUDE_DIR)
    SET( Boost_FOUND TRUE )

    # Check the version of Boost against the requested version.
    if (Boost_FIND_VERSION AND NOT Boost_FIND_VERSION_MINOR)
      message(SEND_ERROR "When requesting a specific version of Boost, you must provide at least the major and minor version numbers, e.g., 1.34")
    endif (Boost_FIND_VERSION AND NOT Boost_FIND_VERSION_MINOR)
    if(Boost_MAJOR_VERSION LESS "${Boost_FIND_VERSION_MAJOR}" )
      set( Boost_FOUND FALSE )
      set(_Boost_VERSION_AGE "old")
    elseif(Boost_MAJOR_VERSION EQUAL "${Boost_FIND_VERSION_MAJOR}" )
      if(Boost_MINOR_VERSION LESS "${Boost_FIND_VERSION_MINOR}" )
        set( Boost_FOUND FALSE )
        set(_Boost_VERSION_AGE "old")
      elseif(Boost_MINOR_VERSION EQUAL "${Boost_FIND_VERSION_MINOR}" )
        if( Boost_FIND_VERSION_PATCH AND Boost_SUBMINOR_VERSION LESS "${Boost_FIND_VERSION_PATCH}" )
          set( Boost_FOUND FALSE )
          set(_Boost_VERSION_AGE "old")
        endif( Boost_FIND_VERSION_PATCH AND Boost_SUBMINOR_VERSION LESS "${Boost_FIND_VERSION_PATCH}" )
      endif( Boost_MINOR_VERSION LESS "${Boost_FIND_VERSION_MINOR}" )
    endif( Boost_MAJOR_VERSION LESS "${Boost_FIND_VERSION_MAJOR}" )

    if (Boost_FOUND AND Boost_FIND_VERSION_EXACT)
      # If the user requested an exact version of Boost, check
      # that. We already know that the Boost version we have is >= the
      # requested version.
      set(_Boost_VERSION_AGE "new")

      # If the user didn't specify a patchlevel, it's 0.
      if (NOT Boost_FIND_VERSION_PATCH)
        set(Boost_FIND_VERSION_PATCH 0)
      endif (NOT Boost_FIND_VERSION_PATCH)
      
      # We'll set Boost_FOUND true again if we have an exact version match.
      set(Boost_FOUND FALSE)
      if(Boost_MAJOR_VERSION EQUAL "${Boost_FIND_VERSION_MAJOR}" )
        if(Boost_MINOR_VERSION EQUAL "${Boost_FIND_VERSION_MINOR}" )
          if(Boost_SUBMINOR_VERSION EQUAL "${Boost_FIND_VERSION_PATCH}" )
            set( Boost_FOUND TRUE )
          endif(Boost_SUBMINOR_VERSION EQUAL "${Boost_FIND_VERSION_PATCH}" )
        endif( Boost_MINOR_VERSION EQUAL "${Boost_FIND_VERSION_MINOR}" )
      endif( Boost_MAJOR_VERSION EQUAL "${Boost_FIND_VERSION_MAJOR}" )
    endif (Boost_FOUND AND Boost_FIND_VERSION_EXACT)

    if(NOT Boost_FOUND)
      # State that we found a version of Boost that is too new or too old.
      set(Boost_ERROR_REASON
        "${Boost_ERROR_REASON}\nDetected version of Boost is too ${_Boost_VERSION_AGE}. Requested version was ${Boost_FIND_VERSION_MAJOR}.${Boost_FIND_VERSION_MINOR}")
      if (Boost_FIND_VERSION_PATCH)
        set(Boost_ERROR_REASON 
          "${Boost_ERROR_REASON}.${Boost_FIND_VERSION_PATCH}")
      endif (Boost_FIND_VERSION_PATCH)
      if (NOT Boost_FIND_VERSION_EXACT)
        set(Boost_ERROR_REASON "${Boost_ERROR_REASON} (or newer)")
      endif (NOT Boost_FIND_VERSION_EXACT)
      set(Boost_ERROR_REASON "${Boost_ERROR_REASON}.")
    endif (NOT Boost_FOUND)

    if (Boost_FOUND)
      set(_boost_CHECKED_COMPONENT FALSE)
      set(_Boost_MISSING_COMPONENTS)
      foreach(COMPONENT ${Boost_FIND_COMPONENTS})
        string(TOUPPER ${COMPONENT} COMPONENT)
        set(_boost_CHECKED_COMPONENT TRUE)
        if(NOT Boost_${COMPONENT}_FOUND)
          string(TOLOWER ${COMPONENT} COMPONENT)
          list(APPEND _Boost_MISSING_COMPONENTS ${COMPONENT})
          set( Boost_FOUND FALSE)
        endif(NOT Boost_${COMPONENT}_FOUND)
      endforeach(COMPONENT)
    endif (Boost_FOUND)

    if (_Boost_MISSING_COMPONENTS)
      # We were unable to find some libraries, so generate a sensible
      # error message that lists the libraries we were unable to find.
      set(Boost_ERROR_REASON
        "${Boost_ERROR_REASON}\nThe following Boost libraries could not be found:\n")
      foreach(COMPONENT ${_Boost_MISSING_COMPONENTS})
        set(Boost_ERROR_REASON
          "${Boost_ERROR_REASON}        boost_${COMPONENT}\n")
      endforeach(COMPONENT)

      list(LENGTH Boost_FIND_COMPONENTS Boost_NUM_COMPONENTS_WANTED)
      list(LENGTH _Boost_MISSING_COMPONENTS Boost_NUM_MISSING_COMPONENTS)
      if (${Boost_NUM_COMPONENTS_WANTED} EQUAL ${Boost_NUM_MISSING_COMPONENTS})
        set(Boost_ERROR_REASON
          "${Boost_ERROR_REASON}No Boost libraries were found. You may need to set Boost_LIBRARYDIR to the directory containing Boost libraries or BOOST_ROOT to the location of Boost.")
      else (${Boost_NUM_COMPONENTS_WANTED} EQUAL ${Boost_NUM_MISSING_COMPONENTS})
        set(Boost_ERROR_REASON
          "${Boost_ERROR_REASON}Some (but not all) of the required Boost libraries were found. You may need to install these additional Boost libraries. Alternatively, set Boost_LIBRARYDIR to the directory containing Boost libraries or BOOST_ROOT to the location of Boost.")
      endif (${Boost_NUM_COMPONENTS_WANTED} EQUAL ${Boost_NUM_MISSING_COMPONENTS})
    endif (_Boost_MISSING_COMPONENTS)

    IF( NOT Boost_LIBRARY_DIRS AND NOT _boost_CHECKED_COMPONENT )
      # Compatibility Code for backwards compatibility with CMake
      # 2.4's FindBoost module.

      # Look for the boost library path.
      # Note that the user may not have installed any libraries
      # so it is quite possible the Boost_LIBRARY_PATH may not exist.
      SET(_boost_LIB_DIR ${Boost_INCLUDE_DIR})
    
      IF("${_boost_LIB_DIR}" MATCHES "boost-[0-9]+")
        GET_FILENAME_COMPONENT(_boost_LIB_DIR ${_boost_LIB_DIR} PATH)
      ENDIF ("${_boost_LIB_DIR}" MATCHES "boost-[0-9]+")
    
      IF("${_boost_LIB_DIR}" MATCHES "/include$")
        # Strip off the trailing "/include" in the path.
        GET_FILENAME_COMPONENT(_boost_LIB_DIR ${_boost_LIB_DIR} PATH)
      ENDIF("${_boost_LIB_DIR}" MATCHES "/include$")
    
      IF(EXISTS "${_boost_LIB_DIR}/lib")
        SET (_boost_LIB_DIR ${_boost_LIB_DIR}/lib)
      ELSE(EXISTS "${_boost_LIB_DIR}/lib")
        IF(EXISTS "${_boost_LIB_DIR}/stage/lib")
          SET(_boost_LIB_DIR ${_boost_LIB_DIR}/stage/lib)
        ELSE(EXISTS "${_boost_LIB_DIR}/stage/lib")
          SET(_boost_LIB_DIR "")
        ENDIF(EXISTS "${_boost_LIB_DIR}/stage/lib")
      ENDIF(EXISTS "${_boost_LIB_DIR}/lib")
    
      IF(_boost_LIB_DIR AND EXISTS "${_boost_LIB_DIR}")
        SET(Boost_LIBRARY_DIRS ${_boost_LIB_DIR} CACHE FILEPATH "Boost library directory")
      ENDIF(_boost_LIB_DIR AND EXISTS "${_boost_LIB_DIR}")

    ENDIF( NOT Boost_LIBRARY_DIRS AND NOT _boost_CHECKED_COMPONENT )

  ELSE(Boost_INCLUDE_DIR)
    SET( Boost_FOUND FALSE)
  ENDIF(Boost_INCLUDE_DIR)

  IF (Boost_FOUND)
      IF (NOT Boost_FIND_QUIETLY)
        MESSAGE(STATUS "Boost version: ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
      ENDIF(NOT Boost_FIND_QUIETLY)
      IF (NOT Boost_FIND_QUIETLY)
        MESSAGE(STATUS "Found the following Boost libraries:")
      ENDIF(NOT Boost_FIND_QUIETLY)
      FOREACH ( COMPONENT  ${Boost_FIND_COMPONENTS} )
        STRING( TOUPPER ${COMPONENT} UPPERCOMPONENT )
        IF ( Boost_${UPPERCOMPONENT}_FOUND )
          IF (NOT Boost_FIND_QUIETLY)
            MESSAGE (STATUS "  ${COMPONENT}")
          ENDIF(NOT Boost_FIND_QUIETLY)
          SET(Boost_LIBRARIES ${Boost_LIBRARIES} ${Boost_${UPPERCOMPONENT}_LIBRARY})
        ENDIF ( Boost_${UPPERCOMPONENT}_FOUND )
      ENDFOREACH(COMPONENT)
  ELSE (Boost_FOUND)
      IF (Boost_FIND_REQUIRED)
        message(SEND_ERROR "Unable to find the requested Boost libraries.\n${Boost_ERROR_REASON}")
      ENDIF(Boost_FIND_REQUIRED)
  ENDIF(Boost_FOUND)

  # Under Windows, automatic linking is performed, so no need to specify the libraries.
  IF (WIN32)
    IF (NOT MINGW)
      SET(Boost_LIBRARIES "")
    ENDIF (NOT MINGW)
  ENDIF(WIN32)

  # show the Boost_INCLUDE_DIRS AND Boost_LIBRARIES variables only in the advanced view
  MARK_AS_ADVANCED(Boost_INCLUDE_DIR
      Boost_INCLUDE_DIRS
      Boost_LIBRARY_DIRS
      Boost_USE_MULTITHREADED
  )
ENDIF(_boost_IN_CACHE)

