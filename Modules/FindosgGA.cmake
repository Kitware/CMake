# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindosgGA
---------



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

Locate osgGA This module defines:

``OSGGA_FOUND``
  Was osgGA found?
``OSGGA_INCLUDE_DIR``
  Where to find the headers
``OSGGA_LIBRARIES``
  The libraries to link against for the osgGA (use this)
``OSGGA_LIBRARY``
  The osgGA library
``OSGGA_LIBRARY_DEBUG``
  The osgGA debug library

``$OSGDIR`` is an environment variable that would correspond to::

  ./configure --prefix=$OSGDIR

used in building osg.

Created by Eric Wing.
#]=======================================================================]

# Header files are presumed to be included like
# #include <osg/PositionAttitudeTransform>
# #include <osgGA/FlightManipulator>

include(${CMAKE_CURRENT_LIST_DIR}/Findosg_functions.cmake)
OSG_FIND_PATH   (OSGGA osgGA/FlightManipulator)
OSG_FIND_LIBRARY(OSGGA osgGA)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(osgGA DEFAULT_MSG
    OSGGA_LIBRARY OSGGA_INCLUDE_DIR)
