# - Find the ImageMagick binary suite.
# This module will search for a set of ImageMagick tools specified
# as components in the FIND_PACKAGE call. Typical components include,
# but are not limited to (future versions of ImageMagick might have
# additional components not listed here):
#
#  animate
#  compare
#  composite
#  conjure
#  convert
#  display
#  identify
#  import
#  mogrify
#  montage
#  stream
#
# If no component is specified in the FIND_PACKAGE call, then it only
# searches for the ImageMagick executable directory. This code defines
# the following variables:
#
#  ImageMagick_FOUND                  - TRUE if all components are found.
#  ImageMagick_EXECUTABLE_DIR         - Full path to executables directory.
#  ImageMagick_<component>_FOUND      - TRUE if <component> is found.
#  ImageMagick_<component>_EXECUTABLE - Full path to <component> executable.
#  ImageMagick_VERSION_STRING         - the version of ImageMagick found
#                                       (since CMake 2.8.8)
#
# ImageMagick_VERSION_STRING will not work for old versions like 5.2.3.
#
# There are also components for the following ImageMagick APIs:
#
#  Magick++
#  MagickWand
#  MagickCore
#
# For these components the following variables are set:
#
#  ImageMagick_FOUND                    - TRUE if all components are found.
#  ImageMagick_INCLUDE_DIRS             - Full paths to all include dirs.
#  ImageMagick_LIBRARIES                - Full paths to all libraries.
#  ImageMagick_<component>_FOUND        - TRUE if <component> is found.
#  ImageMagick_<component>_INCLUDE_DIRS - Full path to <component> include dirs.
#  ImageMagick_<component>_LIBRARIES    - Full path to <component> libraries.
#
# Example Usages:
#  FIND_PACKAGE(ImageMagick)
#  FIND_PACKAGE(ImageMagick COMPONENTS convert)
#  FIND_PACKAGE(ImageMagick COMPONENTS convert mogrify display)
#  FIND_PACKAGE(ImageMagick COMPONENTS Magick++)
#  FIND_PACKAGE(ImageMagick COMPONENTS Magick++ convert)
#
# Note that the standard FIND_PACKAGE features are supported
# (i.e., QUIET, REQUIRED, etc.).

#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
# Copyright 2007-2008 Miguel A. Figueroa-Villanueva <miguelf at ieee dot org>
# Copyright 2012 Rolf Eike Beer <eike@sf-mail.de>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

#---------------------------------------------------------------------
# Helper functions
#---------------------------------------------------------------------
FUNCTION(FIND_IMAGEMAGICK_API component header)
  SET(ImageMagick_${component}_FOUND FALSE PARENT_SCOPE)

  FIND_PATH(ImageMagick_${component}_INCLUDE_DIR
    NAMES ${header}
    PATHS
      ${ImageMagick_INCLUDE_DIRS}
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;BinPath]/include"
    PATH_SUFFIXES
      ImageMagick
    DOC "Path to the ImageMagick include dir."
    )
  FIND_LIBRARY(ImageMagick_${component}_LIBRARY
    NAMES ${ARGN}
    PATHS
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;BinPath]/lib"
    DOC "Path to the ImageMagick Magick++ library."
    )

  IF(ImageMagick_${component}_INCLUDE_DIR AND ImageMagick_${component}_LIBRARY)
    SET(ImageMagick_${component}_FOUND TRUE PARENT_SCOPE)

    LIST(APPEND ImageMagick_INCLUDE_DIRS
      ${ImageMagick_${component}_INCLUDE_DIR}
      )
    LIST(REMOVE_DUPLICATES ImageMagick_INCLUDE_DIRS)
    SET(ImageMagick_INCLUDE_DIRS ${ImageMagick_INCLUDE_DIRS} PARENT_SCOPE)

    LIST(APPEND ImageMagick_LIBRARIES
      ${ImageMagick_${component}_LIBRARY}
      )
    SET(ImageMagick_LIBRARIES ${ImageMagick_LIBRARIES} PARENT_SCOPE)
  ENDIF(ImageMagick_${component}_INCLUDE_DIR AND ImageMagick_${component}_LIBRARY)
ENDFUNCTION(FIND_IMAGEMAGICK_API)

FUNCTION(FIND_IMAGEMAGICK_EXE component)
  SET(_IMAGEMAGICK_EXECUTABLE
    ${ImageMagick_EXECUTABLE_DIR}/${component}${CMAKE_EXECUTABLE_SUFFIX})
  IF(EXISTS ${_IMAGEMAGICK_EXECUTABLE})
    SET(ImageMagick_${component}_EXECUTABLE
      ${_IMAGEMAGICK_EXECUTABLE}
       PARENT_SCOPE
       )
    SET(ImageMagick_${component}_FOUND TRUE PARENT_SCOPE)
  ELSE(EXISTS ${_IMAGEMAGICK_EXECUTABLE})
    SET(ImageMagick_${component}_FOUND FALSE PARENT_SCOPE)
  ENDIF(EXISTS ${_IMAGEMAGICK_EXECUTABLE})
ENDFUNCTION(FIND_IMAGEMAGICK_EXE)

#---------------------------------------------------------------------
# Start Actual Work
#---------------------------------------------------------------------
# Try to find a ImageMagick installation binary path.
FIND_PATH(ImageMagick_EXECUTABLE_DIR
  NAMES mogrify${CMAKE_EXECUTABLE_SUFFIX}
  PATHS
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;BinPath]"
  DOC "Path to the ImageMagick binary directory."
  NO_DEFAULT_PATH
  )
FIND_PATH(ImageMagick_EXECUTABLE_DIR
  NAMES mogrify${CMAKE_EXECUTABLE_SUFFIX}
  )

# Find each component. Search for all tools in same dir
# <ImageMagick_EXECUTABLE_DIR>; otherwise they should be found
# independently and not in a cohesive module such as this one.
UNSET(ImageMagick_REQUIRED_VARS)
UNSET(ImageMagick_DEFAULT_EXECUTABLES)
FOREACH(component ${ImageMagick_FIND_COMPONENTS}
    # DEPRECATED: forced components for backward compatibility
    convert mogrify import montage composite
    )
  IF(component STREQUAL "Magick++")
    FIND_IMAGEMAGICK_API(Magick++ Magick++.h
      Magick++ CORE_RL_Magick++_
      )
    LIST(APPEND ImageMagick_REQUIRED_VARS ImageMagick_Magick++_LIBRARY)
  ELSEIF(component STREQUAL "MagickWand")
    FIND_IMAGEMAGICK_API(MagickWand wand/MagickWand.h
      Wand MagickWand CORE_RL_wand_
      )
    LIST(APPEND ImageMagick_REQUIRED_VARS ImageMagick_MagickWand_LIBRARY)
  ELSEIF(component STREQUAL "MagickCore")
    FIND_IMAGEMAGICK_API(MagickCore magick/MagickCore.h
      Magick MagickCore CORE_RL_magick_
      )
    LIST(APPEND ImageMagick_REQUIRED_VARS ImageMagick_MagickCore_LIBRARY)
  ELSE(component STREQUAL "Magick++")
    IF(ImageMagick_EXECUTABLE_DIR)
      FIND_IMAGEMAGICK_EXE(${component})
    ENDIF(ImageMagick_EXECUTABLE_DIR)

    IF(ImageMagick_FIND_COMPONENTS)
      LIST(FIND ImageMagick_FIND_COMPONENTS ${component} is_requested)
      IF(is_requested GREATER -1)
        LIST(APPEND ImageMagick_REQUIRED_VARS ImageMagick_${component}_EXECUTABLE)
      ENDIF(is_requested GREATER -1)
    ELSEIF(ImageMagick_${component}_EXECUTABLE)
      # if no components were requested explicitly put all (default) executables
      # in the list
      LIST(APPEND ImageMagick_DEFAULT_EXECUTABLES ImageMagick_${component}_EXECUTABLE)
    ENDIF(ImageMagick_FIND_COMPONENTS)
  ENDIF(component STREQUAL "Magick++")
ENDFOREACH(component)

IF(NOT ImageMagick_FIND_COMPONENTS AND NOT ImageMagick_DEFAULT_EXECUTABLES)
  # No components were requested, and none of the default components were
  # found. Just insert mogrify into the list of the default components to
  # find so FPHSA below has something to check
  LIST(APPEND ImageMagick_REQUIRED_VARS ImageMagick_mogrify_EXECUTABLE)
ELSEIF(ImageMagick_DEFAULT_EXECUTABLES)
  LIST(APPEND ImageMagick_REQUIRED_VARS ${ImageMagick_DEFAULT_EXECUTABLES})
ENDIF()

SET(ImageMagick_INCLUDE_DIRS ${ImageMagick_INCLUDE_DIRS})
SET(ImageMagick_LIBRARIES ${ImageMagick_LIBRARIES})

IF(ImageMagick_mogrify_EXECUTABLE)
  EXECUTE_PROCESS(COMMAND ${ImageMagick_mogrify_EXECUTABLE} -version
                  OUTPUT_VARIABLE imagemagick_version
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  IF(imagemagick_version MATCHES "^Version: ImageMagick [0-9]")
    STRING(REGEX REPLACE "^Version: ImageMagick ([-0-9\\.]+).*" "\\1" ImageMagick_VERSION_STRING "${imagemagick_version}")
  ENDIF()
  UNSET(imagemagick_version)
ENDIF(ImageMagick_mogrify_EXECUTABLE)

#---------------------------------------------------------------------
# Standard Package Output
#---------------------------------------------------------------------
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ImageMagick
                                  REQUIRED_VARS ${ImageMagick_REQUIRED_VARS}
                                  VERSION_VAR ImageMagick_VERSION_STRING
  )
# Maintain consistency with all other variables.
SET(ImageMagick_FOUND ${IMAGEMAGICK_FOUND})

#---------------------------------------------------------------------
# DEPRECATED: Setting variables for backward compatibility.
#---------------------------------------------------------------------
SET(IMAGEMAGICK_BINARY_PATH          ${ImageMagick_EXECUTABLE_DIR}
    CACHE PATH "Path to the ImageMagick binary directory.")
SET(IMAGEMAGICK_CONVERT_EXECUTABLE   ${ImageMagick_convert_EXECUTABLE}
    CACHE FILEPATH "Path to ImageMagick's convert executable.")
SET(IMAGEMAGICK_MOGRIFY_EXECUTABLE   ${ImageMagick_mogrify_EXECUTABLE}
    CACHE FILEPATH "Path to ImageMagick's mogrify executable.")
SET(IMAGEMAGICK_IMPORT_EXECUTABLE    ${ImageMagick_import_EXECUTABLE}
    CACHE FILEPATH "Path to ImageMagick's import executable.")
SET(IMAGEMAGICK_MONTAGE_EXECUTABLE   ${ImageMagick_montage_EXECUTABLE}
    CACHE FILEPATH "Path to ImageMagick's montage executable.")
SET(IMAGEMAGICK_COMPOSITE_EXECUTABLE ${ImageMagick_composite_EXECUTABLE}
    CACHE FILEPATH "Path to ImageMagick's composite executable.")
MARK_AS_ADVANCED(
  IMAGEMAGICK_BINARY_PATH
  IMAGEMAGICK_CONVERT_EXECUTABLE
  IMAGEMAGICK_MOGRIFY_EXECUTABLE
  IMAGEMAGICK_IMPORT_EXECUTABLE
  IMAGEMAGICK_MONTAGE_EXECUTABLE
  IMAGEMAGICK_COMPOSITE_EXECUTABLE
  )
