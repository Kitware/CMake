
#=============================================================================
# Copyright 2004-2011 Kitware, Inc.
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

# This should be included before the _INIT variables are
# used to initialize the cache.  Since the rule variables 
# have if blocks on them, users can still define them here.
# But, it should still be after the platform file so changes can
# be made to those values.

IF(CMAKE_USER_MAKE_RULES_OVERRIDE)
  # Save the full path of the file so try_compile can use it.
  INCLUDE(${CMAKE_USER_MAKE_RULES_OVERRIDE} RESULT_VARIABLE _override)
  SET(CMAKE_USER_MAKE_RULES_OVERRIDE "${_override}")
ENDIF()

IF(CMAKE_USER_MAKE_RULES_OVERRIDE_Java)
  # Save the full path of the file so try_compile can use it.
   INCLUDE(${CMAKE_USER_MAKE_RULES_OVERRIDE_Java} RESULT_VARIABLE _override)
   SET(CMAKE_USER_MAKE_RULES_OVERRIDE_Java "${_override}")
ENDIF()

# this is a place holder if java needed flags for javac they would go here.
IF(NOT CMAKE_Java_CREATE_STATIC_LIBRARY)
#  IF(WIN32)
#    SET(class_files_mask "*.class")
#  ELSE(WIN32)
    SET(class_files_mask ".")
#  ENDIF(WIN32)

  SET(CMAKE_Java_CREATE_STATIC_LIBRARY
      "<CMAKE_Java_ARCHIVE> -cf <TARGET> -C <OBJECT_DIR> ${class_files_mask}")
    # "${class_files_mask}" should really be "<OBJECTS>" but compling a *.java
    # file can create more than one *.class file...
ENDIF(NOT CMAKE_Java_CREATE_STATIC_LIBRARY)

# compile a Java file into an object file
IF(NOT CMAKE_Java_COMPILE_OBJECT)
  SET(CMAKE_Java_COMPILE_OBJECT
    "<CMAKE_Java_COMPILER> <FLAGS> <SOURCE> -d <OBJECT_DIR>")
ENDIF(NOT CMAKE_Java_COMPILE_OBJECT)

# set java include flag option and the separator for multiple include paths
SET(CMAKE_INCLUDE_FLAG_Java "-classpath ")
IF(WIN32 AND NOT CYGWIN)
  SET(CMAKE_INCLUDE_FLAG_SEP_Java ";")
ELSE(WIN32 AND NOT CYGWIN)
  SET(CMAKE_INCLUDE_FLAG_SEP_Java ":")
ENDIF(WIN32 AND NOT CYGWIN)
