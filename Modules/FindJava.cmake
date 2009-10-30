# - Find Java
# This module finds if Java is installed and determines where the
# include files and libraries are. This code sets the following
# variables:
#
#  Java_JAVA_EXECUTABLE    = the full path to the Java runtime
#  Java_JAVA_EXECUTABLE    = the full path to the Java compiler
#  Java_JAVA_EXECUTABLE    = the full path to the Java archiver
#  Java_VERSION_STRING     = Version of the package found (java version)
#  Java_VERSION_MAJOR      = The major version of the package found.
#  Java_VERSION_MINOR      = The minor version of the package found.
#  Java_VERSION_PATCH      = The patch version of the package found.
#                            The patch version may contains underscore '_'
#

#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
# Copyright 2009 Mathieu Malaterre <mathieu.malaterre@gmail.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

SET(JAVA_BIN_PATH
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\2.0;JavaHome]/bin"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.9;JavaHome]/bin"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.8;JavaHome]/bin"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.7;JavaHome]/bin"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.6;JavaHome]/bin"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.5;JavaHome]/bin"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.4;JavaHome]/bin"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.3;JavaHome]/bin"
  $ENV{JAVA_HOME}/bin
  /usr/bin
  /usr/lib/java/bin
  /usr/share/java/bin
  /usr/local/bin
  /usr/local/java/bin
  /usr/local/java/share/bin
  /usr/java/j2sdk1.4.2_04
  /usr/lib/j2sdk1.4-sun/bin
  /usr/java/j2sdk1.4.2_09/bin
  /usr/lib/j2sdk1.5-sun/bin
  /opt/sun-jdk-1.5.0.04/bin
  )
FIND_PROGRAM(Java_JAVA_EXECUTABLE
  NAMES java
  PATHS ${JAVA_BIN_PATH}
  NO_DEFAULT_PATH
)

IF(Java_JAVA_EXECUTABLE)
    EXECUTE_PROCESS(COMMAND ${Java_JAVA_EXECUTABLE} -version
      RESULT_VARIABLE res
      OUTPUT_VARIABLE var
      ERROR_VARIABLE var # sun-java output to stderr
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_STRIP_TRAILING_WHITESPACE)
    IF( res )
      MESSAGE( FATAL_ERROR "Error executing java -version" )
    ELSE()
        # extract major/minor version and patch level from "java -version" output
        # Tested on linux using 
        # 1. Sun
        # 2. OpenJDK 1.6
        # 3. GCJ 1.5
        STRING( REGEX REPLACE ".* version \"([0-9]+\\.[0-9]+\\.[0-9_]+).*"
                "\\1" Java_VERSION_STRING "${var}" )
        STRING( REGEX REPLACE ".* version \"([0-9]+)\\.[0-9]+\\.[0-9_]+.*"
                "\\1" Java_VERSION_MAJOR "${var}" )
        STRING( REGEX REPLACE ".* version \"[0-9]+\\.([0-9]+)\\.[0-9_]+.*"
                "\\1" Java_VERSION_MINOR "${var}" )
        STRING( REGEX REPLACE ".* version \"[0-9]+\\.[0-9]+\\.([0-9_]+).*"
                "\\1" Java_VERSION_PATCH "${var}" )
        # display info
        MESSAGE( STATUS "Java version ${Java_VERSION_STRING} configured successfully!" )
        MESSAGE( STATUS "Java version ${Java_VERSION_MAJOR}.${Java_VERSION_MINOR} configured successfully!" )
    ENDIF()

    # If any version numbers are given to the command it will set the
    # following variables before loading the module:
    #
    #  XXX_FIND_VERSION       = full requested version string
    #  XXX_FIND_VERSION_MAJOR = major version if requested, else 0
    #  XXX_FIND_VERSION_MINOR = minor version if requested, else 0
    #  XXX_FIND_VERSION_PATCH = patch version if requested, else 0
    #  XXX_FIND_VERSION_TWEAK = tweak version if requested, else 0
    #  XXX_FIND_VERSION_COUNT = number of version components, 0 to 4
    #  XXX_FIND_VERSION_EXACT = true if EXACT option was given
    set(_java_version_acceptable TRUE)
    if( Java_FIND_VERSION )
      if( Java_FIND_VERSION_MAJOR GREATER Java_VERSION_MAJOR )
        set(_java_version_acceptable FALSE)
      endif( Java_FIND_VERSION_MAJOR  GREATER Java_VERSION_MAJOR )
      if( Java_FIND_VERSION_MINOR GREATER Java_VERSION_MINOR )
        set(_java_version_acceptable FALSE)
      endif( Java_FIND_VERSION_MINOR  GREATER Java_VERSION_MINOR )
      # Is it exact ?
      if( Java_FIND_VERSION_EXACT )
        # since GREATER operation worked ok, simply check LESS operation
        if( Java_FIND_VERSION_MAJOR LESS Java_VERSION_MAJOR )
          set(_java_version_acceptable FALSE)
        endif( Java_FIND_VERSION_MAJOR  LESS Java_VERSION_MAJOR )
        if( Java_FIND_VERSION_MINOR LESS Java_VERSION_MINOR )
          set(_java_version_acceptable FALSE)
        endif( Java_FIND_VERSION_MINOR LESS Java_VERSION_MINOR )
      endif( Java_FIND_VERSION_EXACT )
    else( Java_FIND_VERSION )
      # no version requested we are all set
    endif( Java_FIND_VERSION )

ENDIF(Java_JAVA_EXECUTABLE)


FIND_PROGRAM(Java_JAR_EXECUTABLE
  NAMES jar
  PATHS ${JAVA_BIN_PATH}
  NO_DEFAULT_PATH
)

FIND_PROGRAM(Java_JAVAC_EXECUTABLE
  NAMES javac
  PATHS ${JAVA_BIN_PATH}
  NO_DEFAULT_PATH
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Java DEFAULT_MSG
  Java_JAVA_EXECUTABLE
  Java_JAR_EXECUTABLE
  Java_JAVAC_EXECUTABLE
  _java_version_acceptable
)


# LEGACY
SET(JAVA_RUNTIME ${Java_JAVA_EXECUTABLE})
SET(JAVA_ARCHIVE ${Java_JAR_EXECUTABLE})
SET(JAVA_COMPILE ${Java_JAVAC_EXECUTABLE})

