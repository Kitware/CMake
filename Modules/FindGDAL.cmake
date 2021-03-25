# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindGDAL
--------

Find Geospatial Data Abstraction Library (GDAL).

IMPORTED Targets
^^^^^^^^^^^^^^^^

.. versionadded:: 3.14

This module defines :prop_tgt:`IMPORTED` target ``GDAL::GDAL``
if GDAL has been found.

Result Variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``GDAL_FOUND``
  True if GDAL is found.
``GDAL_INCLUDE_DIRS``
  Include directories for GDAL headers.
``GDAL_LIBRARIES``
  Libraries to link to GDAL.
``GDAL_VERSION``
  .. versionadded:: 3.14
    The version of GDAL found.

Cache variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``GDAL_LIBRARY``
  The libgdal library file.
``GDAL_INCLUDE_DIR``
  The directory containing ``gdal.h``.

Hints
^^^^^

Set ``GDAL_DIR`` or ``GDAL_ROOT`` in the environment to specify the
GDAL installation prefix.

The following variables may be set to modify the search strategy:

``FindGDAL_SKIP_GDAL_CONFIG``
  If set, ``gdal-config`` will not be used. This can be useful if there are
  GDAL libraries built with autotools (which provide the tool) and CMake (which
  do not) in the same environment.
``GDAL_ADDITIONAL_LIBRARY_VERSIONS``
  Extra versions of library names to search for.
#]=======================================================================]

# $GDALDIR is an environment variable that would
# correspond to the ./configure --prefix=$GDAL_DIR
# used in building gdal.
#
# Created by Eric Wing. I'm not a gdal user, but OpenSceneGraph uses it
# for osgTerrain so I whipped this module together for completeness.
# I actually don't know the conventions or where files are typically
# placed in distros.
# Any real gdal users are encouraged to correct this (but please don't
# break the OS X framework stuff when doing so which is what usually seems
# to happen).

# This makes the presumption that you are include gdal.h like
#
#include "gdal.h"

find_path(GDAL_INCLUDE_DIR gdal.h
  HINTS
    ENV GDAL_DIR
    ENV GDAL_ROOT
  PATH_SUFFIXES
    include/gdal
    include/GDAL
    include
  DOC "Path to the GDAL include directory"
)
mark_as_advanced(GDAL_INCLUDE_DIR)

if(UNIX AND NOT FindGDAL_SKIP_GDAL_CONFIG)
    # Use gdal-config to obtain the library version (this should hopefully
    # allow us to -lgdal1.x.y where x.y are correct version)
    # For some reason, libgdal development packages do not contain
    # libgdal.so...
    find_program(GDAL_CONFIG gdal-config
        HINTS
          ENV GDAL_DIR
          ENV GDAL_ROOT
        PATH_SUFFIXES bin
        DOC "Path to the gdal-config tool"
    )
    mark_as_advanced(GDAL_CONFIG)

    if(GDAL_CONFIG)
        execute_process(COMMAND ${GDAL_CONFIG} --libs OUTPUT_VARIABLE GDAL_CONFIG_LIBS)

        if(GDAL_CONFIG_LIBS)
            # treat the output as a command line and split it up
            separate_arguments(args NATIVE_COMMAND "${GDAL_CONFIG_LIBS}")

            # only consider libraries whose name matches this pattern
            set(name_pattern "[gG][dD][aA][lL]")

            # consider each entry as a possible library path, name, or parent directory
            foreach(arg IN LISTS args)
                # library name
                if("${arg}" MATCHES "^-l(.*)$")
                    set(lib "${CMAKE_MATCH_1}")

                    # only consider libraries whose name matches the expected pattern
                    if("${lib}" MATCHES "${name_pattern}")
                        list(APPEND _gdal_lib "${lib}")
                    endif()
                # library search path
                elseif("${arg}" MATCHES "^-L(.*)$")
                    list(APPEND _gdal_libpath "${CMAKE_MATCH_1}")
                # assume this is a full path to a library
                elseif(IS_ABSOLUTE "${arg}" AND EXISTS "${arg}")
                    # extract the file name
                    get_filename_component(lib "${arg}" NAME)

                    # only consider libraries whose name matches the expected pattern
                    if(NOT "${lib}" MATCHES "${name_pattern}")
                        continue()
                    endif()

                    # extract the file directory
                    get_filename_component(dir "${arg}" DIRECTORY)

                    # remove library prefixes/suffixes
                    string(REGEX REPLACE "^(${CMAKE_SHARED_LIBRARY_PREFIX}|${CMAKE_STATIC_LIBRARY_PREFIX})" "" lib "${lib}")
                    string(REGEX REPLACE "(${CMAKE_SHARED_LIBRARY_SUFFIX}|${CMAKE_STATIC_LIBRARY_SUFFIX})$" "" lib "${lib}")

                    # use the file name and directory as hints
                    list(APPEND _gdal_libpath "${dir}")
                    list(APPEND _gdal_lib "${lib}")
                endif()
            endforeach()
        endif()
    endif()
endif()

# GDAL name its library when built with CMake as `gdal${major}${minor}`.
set(_gdal_versions
    ${GDAL_ADDITIONAL_LIBRARY_VERSIONS} 3.0 2.4 2.3 2.2 2.1 2.0 1.11 1.10 1.9 1.8 1.7 1.6 1.5 1.4 1.3 1.2)

set(_gdal_libnames)
foreach (_gdal_version IN LISTS _gdal_versions)
    string(REPLACE "." "" _gdal_version "${_gdal_version}")
    list(APPEND _gdal_libnames "gdal${_gdal_version}" "GDAL${_gdal_version}")
endforeach ()
unset(_gdal_version)
unset(_gdal_versions)

find_library(GDAL_LIBRARY
  NAMES ${_gdal_lib} ${_gdal_libnames} gdal gdal_i gdal1.5.0 gdal1.4.0 gdal1.3.2 GDAL
  HINTS
     ENV GDAL_DIR
     ENV GDAL_ROOT
     ${_gdal_libpath}
  PATH_SUFFIXES lib
  DOC "Path to the GDAL library"
)
mark_as_advanced(GDAL_LIBRARY)
unset(_gdal_libnames)
unset(_gdal_lib)

if (EXISTS "${GDAL_INCLUDE_DIR}/gdal_version.h")
    file(STRINGS "${GDAL_INCLUDE_DIR}/gdal_version.h" _gdal_version
        REGEX "GDAL_RELEASE_NAME")
    string(REGEX REPLACE ".*\"\(.*\)\"" "\\1" GDAL_VERSION "${_gdal_version}")
    unset(_gdal_version)
else ()
    set(GDAL_VERSION GDAL_VERSION-NOTFOUND)
endif ()

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GDAL
    VERSION_VAR GDAL_VERSION
    REQUIRED_VARS GDAL_LIBRARY GDAL_INCLUDE_DIR)

if (GDAL_FOUND)
    set(GDAL_LIBRARIES ${GDAL_LIBRARY})
    set(GDAL_INCLUDE_DIRS ${GDAL_INCLUDE_DIR})

    if (NOT TARGET GDAL::GDAL)
        add_library(GDAL::GDAL UNKNOWN IMPORTED)
        set_target_properties(GDAL::GDAL PROPERTIES
            IMPORTED_LOCATION "${GDAL_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GDAL_INCLUDE_DIR}")
    endif ()
endif ()
