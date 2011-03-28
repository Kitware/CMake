# - Find Java
# This module finds if Java is installed and determines where the
# include files and libraries are. This code sets the following
# variables:
#
#  Java_JAVA_EXECUTABLE    = the full path to the Java runtime
#  Java_JAVAC_EXECUTABLE   = the full path to the Java compiler
#  Java_JAR_EXECUTABLE     = the full path to the Java archiver
#  Java_VERSION_STRING     = Version of the package found (java version), eg. 1.6.0_12
#  Java_VERSION_MAJOR      = The major version of the package found.
#  Java_VERSION_MINOR      = The minor version of the package found.
#  Java_VERSION_PATCH      = The patch version of the package found.
#  Java_VERSION_TWEAK      = The tweak version of the package found (after '_')
#  Java_VERSION            = This is set to: $major.$minor.$patch(.$tweak)
#
# The minimum required version of Java can be specified using the
# standard CMake syntax, e.g. FIND_PACKAGE(Java 1.5)
#
# NOTE: ${Java_VERSION} and ${Java_VERSION_STRING} are not guaranteed to be
# identical. For example some java version may return:
# Java_VERSION_STRING = 1.5.0_17
# and
# Java_VERSION        = 1.5.0.17
# 
# another example is the Java OEM, with:
# Java_VERSION_STRING = 1.6.0-oem
# and
# Java_VERSION        = 1.6.0
#
# For these components the following variables are set:
#
#  Java_FOUND                    - TRUE if all components are found.
#  Java_INCLUDE_DIRS             - Full paths to all include dirs.
#  Java_LIBRARIES                - Full paths to all libraries.
#  Java_<component>_FOUND        - TRUE if <component> is found.
#
# Example Usages:
#  FIND_PACKAGE(Java)
#  FIND_PACKAGE(Java COMPONENTS Runtime)
#  FIND_PACKAGE(Java COMPONENTS Development)
#

#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
# Copyright 2009-2011 Mathieu Malaterre <mathieu.malaterre@gmail.com>
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

# The HINTS option should only be used for values computed from the system.
SET(_JAVA_HINTS
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\2.0;JavaHome]/bin"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.9;JavaHome]/bin"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.8;JavaHome]/bin"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.7;JavaHome]/bin"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.6;JavaHome]/bin"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.5;JavaHome]/bin"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.4;JavaHome]/bin"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.3;JavaHome]/bin"
  $ENV{JAVA_HOME}/bin
  )
# Hard-coded guesses should still go in PATHS. This ensures that the user
# environment can always override hard guesses.
SET(_JAVA_PATHS
  /usr/lib/java/bin
  /usr/share/java/bin
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
  HINTS ${_JAVA_HINTS}
  PATHS ${_JAVA_PATHS}
)

IF(Java_JAVA_EXECUTABLE)
    EXECUTE_PROCESS(COMMAND ${Java_JAVA_EXECUTABLE} -version
      RESULT_VARIABLE res
      OUTPUT_VARIABLE var
      ERROR_VARIABLE var # sun-java output to stderr
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_STRIP_TRAILING_WHITESPACE)
    IF( res )
      IF(${Java_FIND_REQUIRED})
        MESSAGE( FATAL_ERROR "Error executing java -version" )
      ELSE()
        MESSAGE( STATUS "Warning, could not run java --version")
      ENDIF()
    ELSE()
      # extract major/minor version and patch level from "java -version" output
      # Tested on linux using 
      # 1. Sun / Sun OEM
      # 2. OpenJDK 1.6
      # 3. GCJ 1.5
      # 4. Kaffe 1.4.2
      IF(var MATCHES "java version \"[0-9]+\\.[0-9]+\\.[0-9_.]+[oem-]*\".*")
        # This is most likely Sun / OpenJDK, or maybe GCJ-java compat layer
        STRING( REGEX REPLACE ".* version \"([0-9]+\\.[0-9]+\\.[0-9_.]+)[oem-]*\".*"
                "\\1" Java_VERSION_STRING "${var}" )
      ELSEIF(var MATCHES "java full version \"kaffe-[0-9]+\\.[0-9]+\\.[0-9_]+\".*")
        # Kaffe style
        STRING( REGEX REPLACE "java full version \"kaffe-([0-9]+\\.[0-9]+\\.[0-9_]+).*"
                "\\1" Java_VERSION_STRING "${var}" )
      ELSE()
        IF(NOT Java_FIND_QUIETLY)
          message(WARNING "regex not supported: ${var}. Please report")
        ENDIF(NOT Java_FIND_QUIETLY)
      ENDIF()
      STRING( REGEX REPLACE "([0-9]+).*" "\\1" Java_VERSION_MAJOR "${Java_VERSION_STRING}" )
      STRING( REGEX REPLACE "[0-9]+\\.([0-9]+).*" "\\1" Java_VERSION_MINOR "${Java_VERSION_STRING}" )
      STRING( REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" Java_VERSION_PATCH "${Java_VERSION_STRING}" )
      # warning tweak version can be empty:
      STRING( REGEX REPLACE "[0-9]+\\.[0-9]+\\.[0-9]+\\_?\\.?([0-9]*)$" "\\1" Java_VERSION_TWEAK "${Java_VERSION_STRING}" )
      if( Java_VERSION_TWEAK STREQUAL "" ) # check case where tweak is not defined
        set(Java_VERSION ${Java_VERSION_MAJOR}.${Java_VERSION_MINOR}.${Java_VERSION_PATCH})
      else( )
        set(Java_VERSION ${Java_VERSION_MAJOR}.${Java_VERSION_MINOR}.${Java_VERSION_PATCH}.${Java_VERSION_TWEAK})
      endif( )
      # display info
      #MESSAGE( STATUS "Java version ${Java_VERSION_STRING} configured successfully!" ) # keep me, used for debug
      IF(NOT Java_FIND_QUIETLY)
        MESSAGE( STATUS "Java version ${Java_VERSION} configured successfully!" )
      ENDIF(NOT Java_FIND_QUIETLY)
    ENDIF()

ENDIF(Java_JAVA_EXECUTABLE)


FIND_PROGRAM(Java_JAR_EXECUTABLE
  NAMES jar
  HINTS ${_JAVA_HINTS}
  PATHS ${_JAVA_PATHS}
)

FIND_PROGRAM(Java_JAVAC_EXECUTABLE
  NAMES javac
  HINTS ${_JAVA_HINTS}
  PATHS ${_JAVA_PATHS}
)

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
if(Java_FIND_COMPONENTS)
  foreach(component ${Java_FIND_COMPONENTS})
    # User just want to execute some Java byte-compiled
    if(component STREQUAL "Runtime")
      find_package_handle_standard_args(Java
        REQUIRED_VARS Java_JAVA_EXECUTABLE
        VERSION_VAR Java_VERSION
        )
    elseif(component STREQUAL "Development")
      find_package_handle_standard_args(Java
        REQUIRED_VARS Java_JAVA_EXECUTABLE Java_JAR_EXECUTABLE Java_JAVAC_EXECUTABLE
        VERSION_VAR Java_VERSION
        )
    else()
      message(FATAL_ERROR "Comp: ${component} is not handled")
    endif()
    set(Java_${component}_FOUND TRUE)
  endforeach(component)
else()
  # Check for everything
  find_package_handle_standard_args(Java
    REQUIRED_VARS Java_JAVA_EXECUTABLE Java_JAR_EXECUTABLE Java_JAVAC_EXECUTABLE
    VERSION_VAR Java_VERSION
    )
endif()


MARK_AS_ADVANCED(
  Java_JAVA_EXECUTABLE
  Java_JAR_EXECUTABLE
  Java_JAVAC_EXECUTABLE
  )

# LEGACY
SET(JAVA_RUNTIME ${Java_JAVA_EXECUTABLE})
SET(JAVA_ARCHIVE ${Java_JAR_EXECUTABLE})
SET(JAVA_COMPILE ${Java_JAVAC_EXECUTABLE})

