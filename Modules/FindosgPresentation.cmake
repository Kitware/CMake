# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindosgPresentation
-------------------



This is part of the ``Findosg*`` suite used to find OpenSceneGraph
components.  Each component is separate and you must opt in to each
module.  You must also opt into OpenGL and OpenThreads (and Producer
if needed) as these modules won't do it for you.  This is to allow you
control over your own system piece by piece in case you need to opt
out of certain components or change the Find behavior for a particular
module (perhaps because the default :module:`FindOpenGL` module doesn't
work with your system as an example).  If you want to use a more
convenient module that includes everything, use the
:module:`FindOpenSceneGraph` instead of the ``Findosg*.cmake`` modules.

Locate osgPresentation This module defines:

``OSGPRESENTATION_FOUND``
  Was osgPresentation found?
``OSGPRESENTATION_INCLUDE_DIR``
  Where to find the headers
``OSGPRESENTATION_LIBRARIES``
  The libraries to link for osgPresentation (use this)
``OSGPRESENTATION_LIBRARY``
  The osgPresentation library
``OSGPRESENTATION_LIBRARY_DEBUG``
  The osgPresentation debug library

``$OSGDIR`` is an environment variable that would correspond to::

  ./configure --prefix=$OSGDIR

used in building osg.
Created by Eric Wing.  Modified to work with osgPresentation by Robert
Osfield, January 2012.
#]=======================================================================]

# Header files are presumed to be included like
# #include <osg/PositionAttitudeTransform>
# #include <osgPresentation/SlideEventHandler>

include(${CMAKE_CURRENT_LIST_DIR}/Findosg_functions.cmake)
OSG_FIND_PATH   (OSGPRESENTATION osgPresentation/SlideEventHandler)
OSG_FIND_LIBRARY(OSGPRESENTATION osgPresentation)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(osgPresentation DEFAULT_MSG
    OSGPRESENTATION_LIBRARY OSGPRESENTATION_INCLUDE_DIR)
