# - Find OpenSceneGraph
# This module searches for the OpenSceneGraph core "osg" library as well as
# OpenThreads, and whatever additional COMPONENTS that you specify.
#    See http://www.openscenegraph.org
#
# NOTE: If you would like to use this module in your CMAKE_MODULE_PATH instead
# of requiring CMake >= 2.6.3, you will also need to download
# FindOpenThreads.cmake, Findosg_functions.cmake, Findosg.cmake, as well as
# files for any Components you need to call (FindosgDB.cmake,
# FindosgUtil.cmake, etc.)
#
#==================================
#
# This module accepts the following variables (note mixed case)
#
#    OpenSceneGraph_DEBUG - Enable debugging output
#
#    OpenSceneGraph_MARK_AS_ADVANCED - Mark cache variables as advanced 
#                                      automatically
#
# The following environment variables are also respected for finding the OSG
# and it's various components.  CMAKE_PREFIX_PATH can also be used for this
# (see find_library() CMake documentation).
#
#    <MODULE>_DIR (where MODULE is of the form "OSGVOLUME" and there is a FindosgVolume.cmake file)
#    OSG_DIR
#    OSGDIR
#    OSG_ROOT
#
# This module defines the following output variables:
#
#    OPENSCENEGRAPH_FOUND - Was the OSG and all of the specified components found?
#
#    OPENSCENEGRAPH_VERSION - The version of the OSG which was found
#
#    OPENSCENEGRAPH_INCLUDE_DIRS - Where to find the headers
#
#    OPENSCENEGRAPH_LIBRARIES - The OSG libraries
#
#==================================
# Example Usage:
#
#  find_package(OpenSceneGraph 2.0.0 COMPONENTS osgDB osgUtil)
#  include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})
#
#  add_executable(foo foo.cc)
#  target_link_libraries(foo ${OPENSCENEGRAPH_LIBRARIES})
#
#==================================
#
# Naming convention:
#  Local variables of the form _osg_foo
#  Input variables of the form OpenSceneGraph_FOO
#  Output variables of the form OPENSCENEGRAPH_FOO
#
# Copyright (c) 2009, Philip Lowman <philip@yhbt.com>
#
# Redistribution AND use is allowed according to the terms of the New
# BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
#==================================

include(Findosg_functions)

set(_osg_modules_to_process)
foreach(_osg_component ${OpenSceneGraph_FIND_COMPONENTS})
    list(APPEND _osg_modules_to_process ${_osg_component})
endforeach()
list(APPEND _osg_modules_to_process "osg" "OpenThreads")
list(REMOVE_DUPLICATES _osg_modules_to_process)

if(OpenSceneGraph_DEBUG)
    message("[ FindOpenSceneGraph.cmake:${CMAKE_CURRENT_LIST_LINE} ] "
        "Components = ${_osg_modules_to_process}")
endif()

#
# First we need to find and parse osg/Version
#
OSG_FIND_PATH(OSG osg/Version)
if(OpenSceneGraph_MARK_AS_ADVANCED)
    OSG_MARK_AS_ADVANCED(OSG)
endif()

# Try to ascertain the version...
if(OSG_INCLUDE_DIR)
    if(OpenSceneGraph_DEBUG)
        message("[ FindOpenSceneGraph.cmake:${CMAKE_CURRENT_LIST_LINE} ] "
            "Detected OSG_INCLUDE_DIR = ${OSG_INCLUDE_DIR}")
    endif()

    file(READ "${OSG_INCLUDE_DIR}/osg/Version" _osg_Version_contents)

    string(REGEX MATCH ".*#define OSG_VERSION_MAJOR[ \t]+[0-9]+.*"
        _osg_old_defines ${_osg_Version_contents})
    string(REGEX MATCH ".*#define OPENSCENEGRAPH_MAJOR_VERSION[ \t]+[0-9]+.*"
        _osg_new_defines ${_osg_Version_contents})
    if(_osg_old_defines)
        string(REGEX REPLACE ".*#define OSG_VERSION_MAJOR[ \t]+([0-9]+).*"
            "\\1" _osg_VERSION_MAJOR ${_osg_Version_contents})
        string(REGEX REPLACE ".*#define OSG_VERSION_MINOR[ \t]+([0-9]+).*"
            "\\1" _osg_VERSION_MINOR ${_osg_Version_contents})
        string(REGEX REPLACE ".*#define OSG_VERSION_PATCH[ \t]+([0-9]+).*"
            "\\1" _osg_VERSION_PATCH ${_osg_Version_contents})
    elseif(_osg_new_defines)
        string(REGEX REPLACE ".*#define OPENSCENEGRAPH_MAJOR_VERSION[ \t]+([0-9]+).*"
            "\\1" _osg_VERSION_MAJOR ${_osg_Version_contents})
        string(REGEX REPLACE ".*#define OPENSCENEGRAPH_MINOR_VERSION[ \t]+([0-9]+).*"
            "\\1" _osg_VERSION_MINOR ${_osg_Version_contents})
        string(REGEX REPLACE ".*#define OPENSCENEGRAPH_PATCH_VERSION[ \t]+([0-9]+).*"
            "\\1" _osg_VERSION_PATCH ${_osg_Version_contents})
    else()
        message("[ FindOpenSceneGraph.cmake:${CMAKE_CURRENT_LIST_LINE} ] "
            "Failed to parse version number, please report this as a bug")
    endif()

    set(OPENSCENEGRAPH_VERSION "${_osg_VERSION_MAJOR}.${_osg_VERSION_MINOR}.${_osg_VERSION_PATCH}"
                                CACHE INTERNAL "The version of OSG which was detected")
    if(OpenSceneGraph_DEBUG)
        message("[ FindOpenSceneGraph.cmake:${CMAKE_CURRENT_LIST_LINE} ] "
            "Detected version ${OPENSCENEGRAPH_VERSION}")
    endif()
endif()

#
# Version checking
#
if(OpenSceneGraph_FIND_VERSION)
    if(OpenSceneGraph_FIND_VERSION_EXACT)
        if(NOT OPENSCENEGRAPH_VERSION VERSION_EQUAL ${OpenSceneGraph_FIND_VERSION})
            set(_osg_version_not_exact TRUE)
        endif()
    else()
        # version is too low
        if(NOT OPENSCENEGRAPH_VERSION VERSION_EQUAL ${OpenSceneGraph_FIND_VERSION} AND 
                NOT OPENSCENEGRAPH_VERSION VERSION_GREATER ${OpenSceneGraph_FIND_VERSION})
            set(_osg_version_not_high_enough TRUE)
        endif()
    endif()
endif()

set(_osg_required)
set(_osg_quiet)
if(OpenSceneGraph_FIND_REQUIRED)
    set(_osg_required "REQUIRED")
endif()
if(OpenSceneGraph_FIND_QUIETLY)
    set(_osg_quiet "QUIET")
endif()
#
# Here we call FIND_PACKAGE() on all of the components
#
foreach(_osg_module ${_osg_modules_to_process})
    if(OpenSceneGraph_DEBUG)
        message("[ FindOpenSceneGraph.cmake:${CMAKE_CURRENT_LIST_LINE} ] "
            "Calling find_package(${_osg_module} ${_osg_required} ${_osg_quiet})")
    endif()
    find_package(${_osg_module} ${_osg_required} ${_osg_quiet})

    string(TOUPPER ${_osg_module} _osg_module_UC)
    list(APPEND OPENSCENEGRAPH_INCLUDE_DIR ${${_osg_module_UC}_INCLUDE_DIR})
    list(APPEND OPENSCENEGRAPH_LIBRARIES ${${_osg_module_UC}_LIBRARIES})

    if(OpenSceneGraph_MARK_AS_ADVANCED)
        OSG_MARK_AS_ADVANCED(${_osg_module})
    endif()
endforeach()

if(OPENSCENEGRAPH_INCLUDE_DIR)
    list(REMOVE_DUPLICATES OPENSCENEGRAPH_INCLUDE_DIR)
endif()
        
#
# Inform the users with an error message based on
# what version they have vs. what version was
# required.
#
if(OpenSceneGraph_FIND_REQUIRED)
    set(_osg_version_output_type FATAL_ERROR)
else()
    set(_osg_version_output_type STATUS)
endif()
if(_osg_version_not_high_enough)
    set(_osg_EPIC_FAIL TRUE)
    if(NOT OpenSceneGraph_FIND_QUIETLY)
        message(${_osg_version_output_type}
            "ERROR: Version ${OpenSceneGraph_FIND_VERSION} or higher of the OSG "
            "is required.  Version ${OPENSCENEGRAPH_VERSION} was found.")
    endif()
elseif(_osg_version_not_exact)
    set(_osg_EPIC_FAIL TRUE)
    if(NOT OpenSceneGraph_FIND_QUIETLY)
        message(${_osg_version_output_type}
            "ERROR: Version ${OpenSceneGraph_FIND_VERSION} of the OSG is required "
            "(exactly), version ${OPENSCENEGRAPH_VERSION} was found.")
    endif()
else()
    # If the version was OK, we should hit this case where we can do the
    # typical user notifications
    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenSceneGraph DEFAULT_MSG OPENSCENEGRAPH_LIBRARIES OPENSCENEGRAPH_INCLUDE_DIR)
endif()

if(_osg_EPIC_FAIL)
    # Zero out everything, we didn't meet version requirements
    set(OPENSCENEGRAPH_FOUND FALSE)
    set(OPENSCENEGRAPH_LIBRARIES)
    set(OPENSCENEGRAPH_INCLUDE_DIR)
endif()

set(OPENSCENEGRAPH_INCLUDE_DIRS ${OPENSCENEGRAPH_INCLUDE_DIR})

