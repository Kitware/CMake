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
#  find_package(ImageMagick)
#  find_package(ImageMagick COMPONENTS convert)
#  find_package(ImageMagick COMPONENTS convert mogrify display)
#  find_package(ImageMagick COMPONENTS Magick++)
#  find_package(ImageMagick COMPONENTS Magick++ convert)
#
# Note that the standard FIND_PACKAGE features are supported
# (i.e., QUIET, REQUIRED, etc.).

#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
# Copyright 2007-2008 Miguel A. Figueroa-Villanueva <miguelf at ieee dot org>
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
function(FIND_IMAGEMAGICK_API component header)
  set(ImageMagick_${component}_FOUND FALSE PARENT_SCOPE)

  find_path(ImageMagick_${component}_INCLUDE_DIR
    NAMES ${header}
    PATHS
      ${ImageMagick_INCLUDE_DIRS}
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;BinPath]/include"
    PATH_SUFFIXES
      ImageMagick
    DOC "Path to the ImageMagick include dir."
    )
  find_library(ImageMagick_${component}_LIBRARY
    NAMES ${ARGN}
    PATHS
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;BinPath]/lib"
    DOC "Path to the ImageMagick Magick++ library."
    )

  if(ImageMagick_${component}_INCLUDE_DIR AND ImageMagick_${component}_LIBRARY)
    set(ImageMagick_${component}_FOUND TRUE PARENT_SCOPE)

    list(APPEND ImageMagick_INCLUDE_DIRS
      ${ImageMagick_${component}_INCLUDE_DIR}
      )
    list(REMOVE_DUPLICATES ImageMagick_INCLUDE_DIRS)
    set(ImageMagick_INCLUDE_DIRS ${ImageMagick_INCLUDE_DIRS} PARENT_SCOPE)

    list(APPEND ImageMagick_LIBRARIES
      ${ImageMagick_${component}_LIBRARY}
      )
    set(ImageMagick_LIBRARIES ${ImageMagick_LIBRARIES} PARENT_SCOPE)
  endif(ImageMagick_${component}_INCLUDE_DIR AND ImageMagick_${component}_LIBRARY)
endfunction(FIND_IMAGEMAGICK_API)

function(FIND_IMAGEMAGICK_EXE component)
  set(_IMAGEMAGICK_EXECUTABLE
    ${ImageMagick_EXECUTABLE_DIR}/${component}${CMAKE_EXECUTABLE_SUFFIX})
  if(EXISTS ${_IMAGEMAGICK_EXECUTABLE})
    set(ImageMagick_${component}_EXECUTABLE
      ${_IMAGEMAGICK_EXECUTABLE}
       PARENT_SCOPE
       )
    set(ImageMagick_${component}_FOUND TRUE PARENT_SCOPE)
  else(EXISTS ${_IMAGEMAGICK_EXECUTABLE})
    set(ImageMagick_${component}_FOUND FALSE PARENT_SCOPE)
  endif(EXISTS ${_IMAGEMAGICK_EXECUTABLE})
endfunction(FIND_IMAGEMAGICK_EXE)

#---------------------------------------------------------------------
# Start Actual Work
#---------------------------------------------------------------------
# Try to find a ImageMagick installation binary path.
find_path(ImageMagick_EXECUTABLE_DIR
  NAMES mogrify${CMAKE_EXECUTABLE_SUFFIX}
  PATHS
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;BinPath]"
  DOC "Path to the ImageMagick binary directory."
  NO_DEFAULT_PATH
  )
find_path(ImageMagick_EXECUTABLE_DIR
  NAMES mogrify${CMAKE_EXECUTABLE_SUFFIX}
  )

# Find each component. Search for all tools in same dir
# <ImageMagick_EXECUTABLE_DIR>; otherwise they should be found
# independently and not in a cohesive module such as this one.
set(ImageMagick_FOUND TRUE)
foreach(component ${ImageMagick_FIND_COMPONENTS}
    # DEPRECATED: forced components for backward compatibility
    convert mogrify import montage composite
    )
  if(component STREQUAL "Magick++")
    FIND_IMAGEMAGICK_API(Magick++ Magick++.h
      Magick++ CORE_RL_Magick++_
      )
  elseif(component STREQUAL "MagickWand")
    FIND_IMAGEMAGICK_API(MagickWand wand/MagickWand.h
      Wand MagickWand CORE_RL_wand_
      )
  elseif(component STREQUAL "MagickCore")
    FIND_IMAGEMAGICK_API(MagickCore magick/MagickCore.h
      Magick MagickCore CORE_RL_magick_
      )
  else(component STREQUAL "Magick++")
    if(ImageMagick_EXECUTABLE_DIR)
      FIND_IMAGEMAGICK_EXE(${component})
    endif(ImageMagick_EXECUTABLE_DIR)
  endif(component STREQUAL "Magick++")

  if(NOT ImageMagick_${component}_FOUND)
    list(FIND ImageMagick_FIND_COMPONENTS ${component} is_requested)
    if(is_requested GREATER -1)
      set(ImageMagick_FOUND FALSE)
    endif(is_requested GREATER -1)
  endif(NOT ImageMagick_${component}_FOUND)
endforeach(component)

set(ImageMagick_INCLUDE_DIRS ${ImageMagick_INCLUDE_DIRS})
set(ImageMagick_LIBRARIES ${ImageMagick_LIBRARIES})

#---------------------------------------------------------------------
# Standard Package Output
#---------------------------------------------------------------------
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  ImageMagick DEFAULT_MSG ImageMagick_FOUND
  )
# Maintain consistency with all other variables.
set(ImageMagick_FOUND ${IMAGEMAGICK_FOUND})

#---------------------------------------------------------------------
# DEPRECATED: Setting variables for backward compatibility.
#---------------------------------------------------------------------
set(IMAGEMAGICK_BINARY_PATH          ${ImageMagick_EXECUTABLE_DIR}
    CACHE PATH "Path to the ImageMagick binary directory.")
set(IMAGEMAGICK_CONVERT_EXECUTABLE   ${ImageMagick_convert_EXECUTABLE}
    CACHE FILEPATH "Path to ImageMagick's convert executable.")
set(IMAGEMAGICK_MOGRIFY_EXECUTABLE   ${ImageMagick_mogrify_EXECUTABLE}
    CACHE FILEPATH "Path to ImageMagick's mogrify executable.")
set(IMAGEMAGICK_IMPORT_EXECUTABLE    ${ImageMagick_import_EXECUTABLE}
    CACHE FILEPATH "Path to ImageMagick's import executable.")
set(IMAGEMAGICK_MONTAGE_EXECUTABLE   ${ImageMagick_montage_EXECUTABLE}
    CACHE FILEPATH "Path to ImageMagick's montage executable.")
set(IMAGEMAGICK_COMPOSITE_EXECUTABLE ${ImageMagick_composite_EXECUTABLE}
    CACHE FILEPATH "Path to ImageMagick's composite executable.")
mark_as_advanced(
  IMAGEMAGICK_BINARY_PATH
  IMAGEMAGICK_CONVERT_EXECUTABLE
  IMAGEMAGICK_MOGRIFY_EXECUTABLE
  IMAGEMAGICK_IMPORT_EXECUTABLE
  IMAGEMAGICK_MONTAGE_EXECUTABLE
  IMAGEMAGICK_COMPOSITE_EXECUTABLE
  )
