#=============================================================================
#
#  Program:   KWSys - Kitware System Library
#  Module:    $RCSfile$
#  Language:  C++
#  Date:      $Date$
#  Version:   $Revision$
#
#  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
#  See http://www.cmake.org/HTML/Copyright.html for details.
#
#     This software is distributed WITHOUT ANY WARRANTY; without even 
#     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
#     PURPOSE.  See the above copyright notices for more information.
#
#=============================================================================

# The Kitware System Library is intended to be included in other
# projects.  It is completely configurable in that the library's
# namespace can be configured and the components that are included
# selected invididually.

# Typical usage is to import the kwsys directory tree into a
# subdirectory under a parent project.  The CMake listfile above this
# one configures the library as follows:
#
#  SET(KWSYS_NAMESPACE fookwsys)
#  SUBDIRS(kwsys)
#
# Optional settings are as follows:
#
#  KWSYS_HEADER_ROOT = The directory into which to generate the kwsys headers.
#                      A directory called "${KWSYS_NAMESPACE}" will be
#                      created under this root directory to hold the files.
#
#    Example:
#
#      SET(KWSYS_HEADER_ROOT ${CMAKE_CURRENT_BINARY_DIR})
#      INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
#
#  KWSYS_LIBRARY_INSTALL_DIR = The installation target directories into
#  KWSYS_HEADER_INSTALL_DIR    which the libraries and headers from
#                              kwsys should be installed by a "make install".
#                              The values should be specified relative to
#                              the installation prefix and start with a '/'.
#    Example:
#
#      SET(KWSYS_LIBRARY_INSTALL_DIR /lib)
#      SET(KWSYS_HEADER_INSTALL_DIR /include)
#
#  KWSYS_FORCE_OLD_STREAMS = Force use of old non-ANSI C++ streams even if
#                            new streams are available.  This may be used
#                            by projects that cannot configure their
#                            streams library.
#    Example:
#
#      SET(KWSYS_FORCE_OLD_STREAMS 1)
#

# Once configured, kwsys should be used as follows from C or C++ code:
#
#  #include <fookwsys/Directory.hxx>
#   ...
#  fookwsys::Directory directory;
#

#-----------------------------------------------------------------------------
# If a namespace is not specified, use "kwsys" and enable testing.
# This should be the case only when kwsys is not included inside
# another project and is being tested.
IF(NOT KWSYS_NAMESPACE)
  SET(KWSYS_NAMESPACE "kwsys")
  SET(KWSYS_STANDALONE 1)
  
  # Enable all components.
  SET(KWSYS_USE_Process 1)
  SET(KWSYS_USE_Directory 1)
  SET(KWSYS_USE_RegularExpression 1)
  SET(KWSYS_USE_SystemTools 1)
ENDIF(NOT KWSYS_NAMESPACE)

#-----------------------------------------------------------------------------
# The project name is that of the specified namespace.
PROJECT(${KWSYS_NAMESPACE})

# Do full dependency headers.
INCLUDE_REGULAR_EXPRESSION("^.*$")

# Work-around for CMake 1.6.7 bug in custom command dependencies when
# there is no executable output path.
IF(NOT EXECUTABLE_OUTPUT_PATH)
  SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}" CACHE PATH
      "Output directory for executables.")
ENDIF(NOT EXECUTABLE_OUTPUT_PATH)

#-----------------------------------------------------------------------------
# We require ANSI support from the C compiler.  Add any needed flags.
IF(CMAKE_ANSI_CFLAGS)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")
ENDIF(CMAKE_ANSI_CFLAGS)

#-----------------------------------------------------------------------------
# Configure the standard library header wrappers based on compiler's
# capabilities and parent project's request.
INCLUDE(${CMAKE_ROOT}/Modules/CheckIncludeFileCXX.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/TestForSTDNamespace.cmake)
CHECK_INCLUDE_FILE_CXX("sstream" CMAKE_HAS_ANSI_STRING_STREAM)
CHECK_INCLUDE_FILE_CXX("iosfwd" CMAKE_ANSI_IOSFWD_HEADER)

# Enforce 0/1 as only possible values.  Needed for the means by which
# Configure.hxx is implemented.
IF(CMAKE_NO_STD_NAMESPACE)
  SET(CMAKE_NO_STD_NAMESPACE 1)
ELSE(CMAKE_NO_STD_NAMESPACE)
  SET(CMAKE_NO_STD_NAMESPACE 0)
ENDIF(CMAKE_NO_STD_NAMESPACE)

IF(CMAKE_ANSI_IOSFWD_HEADER)
  SET(CMAKE_NO_ANSI_STREAM_HEADERS 0)
ELSE(CMAKE_ANSI_IOSFWD_HEADER)
  SET(CMAKE_NO_ANSI_STREAM_HEADERS 1)
ENDIF(CMAKE_ANSI_IOSFWD_HEADER)

IF(CMAKE_HAS_ANSI_STRING_STREAM)
  SET(CMAKE_NO_ANSI_STRING_STREAM 0)
ELSE(CMAKE_HAS_ANSI_STRING_STREAM)
  SET(CMAKE_NO_ANSI_STRING_STREAM 1)
ENDIF(CMAKE_HAS_ANSI_STRING_STREAM)

IF(KWSYS_FORCE_OLD_STREAMS)
  SET(CMAKE_NO_ANSI_STREAM_HEADERS 1)
  SET(CMAKE_NO_ANSI_STRING_STREAM 1)
ENDIF(KWSYS_FORCE_OLD_STREAMS)

IF(KWSYS_NAMESPACE MATCHES "^kwsys$")
  SET(KWSYS_NAME_IS_KWSYS 1)
ELSE(KWSYS_NAMESPACE MATCHES "^kwsys$")
  SET(KWSYS_NAME_IS_KWSYS 0)
ENDIF(KWSYS_NAMESPACE MATCHES "^kwsys$")

# Set names that are used in creating Configure.hxx for these test
# results.
SET(KWSYS_NO_STD_NAMESPACE ${CMAKE_NO_STD_NAMESPACE})
SET(KWSYS_NO_ANSI_STREAM_HEADERS ${CMAKE_NO_ANSI_STREAM_HEADERS})
SET(KWSYS_NO_ANSI_STRING_STREAM ${CMAKE_NO_ANSI_STRING_STREAM})

#-----------------------------------------------------------------------------
# Choose a directory for the generated headers.
IF(NOT KWSYS_HEADER_ROOT)
  SET(KWSYS_HEADER_ROOT "${PROJECT_BINARY_DIR}")
ENDIF(NOT KWSYS_HEADER_ROOT)
SET(KWSYS_HEADER_DIR "${KWSYS_HEADER_ROOT}/${KWSYS_NAMESPACE}")

#-----------------------------------------------------------------------------
# Create STL header wrappers to block warnings in the STL headers and
# give standard names by which they may be included.
FOREACH(header algorithm deque iterator list map numeric queue set stack string
               utility vector)
  # Configure the header wrapper.
  SET(KWSYS_STL_HEADER "${header}")
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/kwsys_std.h.in
                 ${KWSYS_HEADER_DIR}/std/${header}
                 @ONLY IMMEDIATE)
  
  # Create an install target for the header wrapper.
  IF(KWSYS_HEADER_INSTALL_DIR)
    INSTALL_FILES(${KWSYS_HEADER_INSTALL_DIR}/${KWSYS_NAMESPACE}
      FILES ${KWSYS_HEADER_DIR}/std/${header})
  ENDIF(KWSYS_HEADER_INSTALL_DIR)
ENDFOREACH(header)

#-----------------------------------------------------------------------------
# Create streams header wrappers to give standard names by which they
# may be included.
FOREACH(header iostream fstream sstream iosfwd)
  # Configure the header wrapper.
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/kwsys_std_${header}.h.in
                 ${KWSYS_HEADER_DIR}/std/${header}
                 @ONLY IMMEDIATE)

  # Create an install target for the header wrapper.
  IF(KWSYS_HEADER_INSTALL_DIR)
    INSTALL_FILES(${KWSYS_HEADER_INSTALL_DIR}/${KWSYS_NAMESPACE}
      FILES ${KWSYS_HEADER_DIR}/std/${header})
  ENDIF(KWSYS_HEADER_INSTALL_DIR)
ENDFOREACH(header)

#-----------------------------------------------------------------------------
# Build a list of classes and headers we need to implement the
# selected components.  Initialize with required components.
SET(KWSYS_CLASSES)
SET(KWSYS_H_FILES)
SET(KWSYS_HXX_FILES Configure)

# Enforce component dependencies (none currently exists).
#IF(KWSYS_USE_SystemTools)
#  SET(KWSYS_USE_Directory 1)
#  SET(KWSYS_USE_RegularExpression 1)
#ENDIF(KWSYS_USE_SystemTools)

# Add selected C++ classes.
FOREACH(c Directory RegularExpression SystemTools)
  IF(KWSYS_USE_${c})
    SET(KWSYS_CLASSES ${KWSYS_CLASSES} ${c})
  ENDIF(KWSYS_USE_${c})
ENDFOREACH(c)

# Add selected C components.
IF(KWSYS_USE_Process)
  SET(KWSYS_H_FILES ${KWSYS_H_FILES} Process)
ENDIF(KWSYS_USE_Process)

#-----------------------------------------------------------------------------
# Build a list of sources for the library based on components that are
# included.
SET(KWSYS_SRCS)

# Add the proper sources for this platform's Process implementation.
IF(KWSYS_USE_Process)
  IF(NOT UNIX)
    # Use the Windows implementation.  We need the encoded forwarding executable.
    SET(KWSYS_SRCS ProcessWin32.c ${CMAKE_CURRENT_BINARY_DIR}/${KWSYS_NAMESPACE}ProcessFwd9xEnc.c)
    SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/${KWSYS_NAMESPACE}ProcessFwd9xEnc.c
                                PROPERTIES GENERATED 1)
  ELSE(NOT UNIX)
    # Use the UNIX implementation.
    SET(KWSYS_SRCS ProcessUNIX.c)
  ENDIF(NOT UNIX)
ENDIF(KWSYS_USE_Process)

# Configure headers of C++ classes and construct the list of sources.
FOREACH(c ${KWSYS_CLASSES})
  # Add this source to the list of source files for the library.
  SET(KWSYS_SRCS ${KWSYS_SRCS} ${c}.cxx)
  
  # Configure the header for this class.
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/${c}.hxx.in ${KWSYS_HEADER_DIR}/${c}.hxx
                 @ONLY IMMEDIATE)

  # Create an install target for the header.
  IF(KWSYS_HEADER_INSTALL_DIR)
    INSTALL_FILES(${KWSYS_HEADER_INSTALL_DIR}/${KWSYS_NAMESPACE}
                  FILES  ${KWSYS_HEADER_DIR}/${c}.hxx)
  ENDIF(KWSYS_HEADER_INSTALL_DIR)
ENDFOREACH(c)

# Configure C headers.
FOREACH(h ${KWSYS_H_FILES})
  # Configure the header into the given directory.
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/${h}.h.in ${KWSYS_HEADER_DIR}/${h}.h
                 @ONLY IMMEDIATE)

  # Create an install target for the header.
  IF(KWSYS_HEADER_INSTALL_DIR)
    INSTALL_FILES(${KWSYS_HEADER_INSTALL_DIR}/${KWSYS_NAMESPACE}
                  FILES  ${KWSYS_HEADER_DIR}/${h}.h)
  ENDIF(KWSYS_HEADER_INSTALL_DIR)
ENDFOREACH(h)

# Configure other C++ headers.
FOREACH(h ${KWSYS_HXX_FILES})
  # Configure the header into the given directory.
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/${h}.hxx.in ${KWSYS_HEADER_DIR}/${h}.hxx
                 @ONLY IMMEDIATE)

  # Create an install target for the header.
  IF(KWSYS_HEADER_INSTALL_DIR)
    INSTALL_FILES(${KWSYS_HEADER_INSTALL_DIR}/${KWSYS_NAMESPACE}
                  FILES  ${KWSYS_HEADER_DIR}/${h}.hxx)
  ENDIF(KWSYS_HEADER_INSTALL_DIR)
ENDFOREACH(h)

#-----------------------------------------------------------------------------
# Add the library with the configured name and list of sources.
ADD_LIBRARY(${KWSYS_NAMESPACE} ${KWSYS_SRCS})

# Create an install target for the library.
IF(KWSYS_LIBRARY_INSTALL_DIR)
  INSTALL_TARGETS(${KWSYS_LIBRARY_INSTALL_DIR} ${KWSYS_NAMESPACE})
ENDIF(KWSYS_LIBRARY_INSTALL_DIR)

# For building kwsys itself, we use a macro defined on the command
# line to configure the namespace in the C and C++ source files.  The
# source files must also include their headers without the namespae
# prefix, so we add the header directory to the include path directly.
ADD_DEFINITIONS("-DKWSYS_NAMESPACE=${KWSYS_NAMESPACE}")
INCLUDE_DIRECTORIES(BEFORE ${KWSYS_HEADER_DIR})

#-----------------------------------------------------------------------------
# Process execution on windows needs to build a forwarding executable
# that works around a Win9x bug.  We encode the executable into a C
# file and build it into the library.  Win9x platforms reproduce the
# executable into a temporary directory when it is needed.
IF(KWSYS_USE_Process)
  IF(NOT UNIX)
    # Build the forwarding executable itself and a program that will
    # encode it into a C file.
    ADD_EXECUTABLE(${KWSYS_NAMESPACE}ProcessFwd9x ProcessFwd9x.c)
    ADD_EXECUTABLE(${KWSYS_NAMESPACE}EncodeExecutable EncodeExecutable.c)

    # Construct the location of the executable to be encoded.
    SET(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})
    IF(EXECUTABLE_OUTPUT_PATH)
      SET(BIN_DIR ${EXECUTABLE_OUTPUT_PATH})
    ENDIF(EXECUTABLE_OUTPUT_PATH)
    
    SET(MAKE_SYSTEM)
    SET(CFG_INTDIR "/${CMAKE_CFG_INTDIR}")
    IF(CMAKE_BUILD_TOOL MATCHES "make")
      SET(CFG_INTDIR "")
    ENDIF(CMAKE_BUILD_TOOL MATCHES "make")

    # Take advantage of a better custom command syntax if possible.
    SET(CMD ${BIN_DIR}${CFG_INTDIR}/${KWSYS_NAMESPACE}EncodeExecutable.exe)
    SET(FWD ${BIN_DIR}${CFG_INTDIR}/${KWSYS_NAMESPACE}ProcessFwd9x.exe)
    IF("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 1.6)
      ADD_CUSTOM_COMMAND(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${KWSYS_NAMESPACE}ProcessFwd9xEnc.c
        COMMAND ${CMD}
        ARGS ${FWD} ${CMAKE_CURRENT_BINARY_DIR}/${KWSYS_NAMESPACE}ProcessFwd9xEnc.c
  	   ${KWSYS_NAMESPACE} ProcessFwd9x
        DEPENDS ${CMD} ${FWD})
    ELSE("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 1.6)
      ADD_CUSTOM_COMMAND(
        TARGET ${KWSYS_NAMESPACE}
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/ProcessFwd9x.c
        COMMAND ${CMD}
        ARGS ${FWD} ${CMAKE_CURRENT_BINARY_DIR}/${KWSYS_NAMESPACE}ProcessFwd9xEnc.c
  	   ${KWSYS_NAMESPACE} ProcessFwd9x
        OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/${KWSYS_NAMESPACE}ProcessFwd9xEnc.c
        DEPENDS ${CMD} ${FWD})
    ENDIF("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 1.6)
    
    # Make sure build occurs in proper order.
    ADD_DEPENDENCIES(${KWSYS_NAMESPACE} ${KWSYS_NAMESPACE}ProcessFwd9x
                     ${KWSYS_NAMESPACE}EncodeExecutable)
  ENDIF(NOT UNIX)
ENDIF(KWSYS_USE_Process)

#-----------------------------------------------------------------------------
# Setup testing if not being built as part of another project.
IF(KWSYS_STANDALONE)
  INCLUDE_DIRECTORIES(${KWSYS_HEADER_ROOT})
  ADD_EXECUTABLE(testProcess testProcess.c)
  ADD_EXECUTABLE(test1 test1.cxx)
  TARGET_LINK_LIBRARIES(testProcess ${KWSYS_NAMESPACE})
  TARGET_LINK_LIBRARIES(test1 ${KWSYS_NAMESPACE})
ENDIF(KWSYS_STANDALONE)
