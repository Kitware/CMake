# This is part of the Findosg* suite used to find OpenSceneGraph components.
# Each component is separate and you must opt in to each module. You must 
# also opt into OpenGL and OpenThreads (and Producer if needed) as these 
# modules won't do it for you. This is to allow you control over your own 
# system piece by piece in case you need to opt out of certain components
# or change the Find behavior for a particular module (perhaps because the
# default FindOpenGL.cmake module doesn't work with your system as an
# example).
# If you want to use a more convenient module that includes everything,
# use the FindOpenSceneGraph.cmake instead of the Findosg*.cmake modules.
# 
# Locate osgIntrospection
# This module defines
# OSGINTROSPECTION_LIBRARY
# OSGINTROSPECTION_FOUND, if false, do not try to link to osgIntrospection
# OSGINTROSPECTION_INCLUDE_DIR, where to find the headers
#
# $OSGDIR is an environment variable that would
# correspond to the ./configure --prefix=$OSGDIR
# used in building osg.
#
# Created by Eric Wing.

# Header files are presumed to be included like
# #include <osg/PositionAttitudeTransform>
# #include <osgIntrospection/Reflection>

# Try the user's environment request before anything else.
FIND_PATH(OSGINTROSPECTION_INCLUDE_DIR osgIntrospection/Reflection
  HINTS
  $ENV{OSGINTROSPECTION_DIR}
  $ENV{OSG_DIR}
  $ENV{OSGDIR}
  PATH_SUFFIXES include
  PATHS
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr
    /sw # Fink
    /opt/local # DarwinPorts
    /opt/csw # Blastwave
    /opt
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OpenThreads_ROOT]
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]
)

FIND_LIBRARY(OSGINTROSPECTION_LIBRARY 
  NAMES osgIntrospection
  HINTS
  $ENV{OSGINTROSPECTION_DIR}
  $ENV{OSG_DIR}
  $ENV{OSGDIR}
  PATH_SUFFIXES lib64 lib
  PATHS
    ~/Library/Frameworks
    /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

SET(OSGINTROSPECTION_FOUND "NO")
IF(OSGINTROSPECTION_LIBRARY AND OSGINTROSPECTION_INCLUDE_DIR)
  SET(OSGINTROSPECTION_FOUND "YES")
ENDIF(OSGINTROSPECTION_LIBRARY AND OSGINTROSPECTION_INCLUDE_DIR)

