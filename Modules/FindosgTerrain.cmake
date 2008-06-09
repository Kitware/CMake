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
# Locate osgTerrain
# This module defines
# OSGTERRAIN_LIBRARY
# OSGTERRAIN_FOUND, if false, do not try to link to osgTerrain
# OSGTERRAIN_INCLUDE_DIR, where to find the headers
#
# $OSGDIR is an environment variable that would
# correspond to the ./configure --prefix=$OSGDIR
# used in building osg.
#
# Created by Eric Wing.

# Header files are presumed to be included like
# #include <osg/PositionAttitudeTransform>
# #include <osgTerrain/Terrain>

# Try the user's environment request before anything else.
FIND_PATH(OSGTERRAIN_INCLUDE_DIR osgTerrain/Terrain
  HINTS
  $ENV{OSGTERRAIN_DIR}
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

FIND_LIBRARY(OSGTERRAIN_LIBRARY 
  NAMES osgTerrain
  HINTS
  $ENV{OSGTERRAIN_DIR}
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

SET(OSGTERRAIN_FOUND "NO")
IF(OSGTERRAIN_LIBRARY AND OSGTERRAIN_INCLUDE_DIR)
  SET(OSGTERRAIN_FOUND "YES")
ENDIF(OSGTERRAIN_LIBRARY AND OSGTERRAIN_INCLUDE_DIR)

