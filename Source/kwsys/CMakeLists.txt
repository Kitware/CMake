#=============================================================================
#
#  Program:   KWSys - Kitware System Library
#  Module:    $RCSfile$
#
#  Copyright (c) Kitware, Inc., Insight Consortium.  All rights reserved.
#  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.
#
#     This software is distributed WITHOUT ANY WARRANTY; without even
#     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#     PURPOSE.  See the above copyright notices for more information.
#
#=============================================================================

# The Kitware System Library is intended to be included in other
# projects.  It is completely configurable in that the library's
# namespace can be configured and the components that are included can
# be selected invididually.

# Typical usage is to import the kwsys directory tree into a
# subdirectory under a parent project and enable the classes that will
# be used.  All classes are disabled by default.  The CMake listfile
# above this one configures the library as follows:
#
#  SET(KWSYS_NAMESPACE foosys)
#  SET(KWSYS_USE_Directory 1)    # Enable Directory class.
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
#      SET(KWSYS_HEADER_ROOT ${PROJECT_BINARY_DIR})
#      INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
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
#  KWSYS_IOS_FORCE_OLD = Force use of old non-ANSI C++ streams even if
#                        new streams are available.  This may be used
#                        by projects that cannot configure their
#                        streams library.
#    Example:
#
#      SET(KWSYS_IOS_FORCE_OLD 1)
#

# Once configured, kwsys should be used as follows from C or C++ code:
#
#  #include <foosys/Directory.hxx>
#   ...
#  foosys::Directory directory;
#

#-----------------------------------------------------------------------------
# If a namespace is not specified, use "kwsys" and enable testing.
# This should be the case only when kwsys is not included inside
# another project and is being tested.
IF(NOT KWSYS_NAMESPACE)
  SET(KWSYS_NAMESPACE "kwsys")
  SET(KWSYS_STANDALONE 1)

  # Enable all components.
  SET(KWSYS_USE_Base64 1)
  SET(KWSYS_USE_Directory 1)
  SET(KWSYS_USE_Process 1)
  SET(KWSYS_USE_RegularExpression 1)
  SET(KWSYS_USE_SystemTools 1)
ENDIF(NOT KWSYS_NAMESPACE)

#-----------------------------------------------------------------------------
# The project name is that of the specified namespace.
PROJECT(${KWSYS_NAMESPACE})

# Enable testing if building standalone.
IF(KWSYS_STANDALONE)
  INCLUDE(${CMAKE_ROOT}/Modules/Dart.cmake)
  MARK_AS_ADVANCED(BUILD_TESTING DART_ROOT TCL_TCLSH)
  IF(BUILD_TESTING)
    ENABLE_TESTING()
  ENDIF(BUILD_TESTING)
ENDIF(KWSYS_STANDALONE)

# Do full dependency headers.
INCLUDE_REGULAR_EXPRESSION("^.*$")

# Work-around for CMake 1.6.7 bug in custom command dependencies when
# there is no executable output path.
IF(NOT EXECUTABLE_OUTPUT_PATH)
  SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}" CACHE PATH
      "Output directory for executables.")
ENDIF(NOT EXECUTABLE_OUTPUT_PATH)

# Generated source files will need this header.
STRING(COMPARE EQUAL "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}"
  KWSYS_IN_SOURCE_BUILD)
IF(NOT KWSYS_IN_SOURCE_BUILD)
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/kwsysPrivate.h
    ${PROJECT_BINARY_DIR}/kwsysPrivate.h COPY_ONLY IMMEDIATE)
ENDIF(NOT KWSYS_IN_SOURCE_BUILD)

#-----------------------------------------------------------------------------
# We require ANSI support from the C compiler.  Add any needed flags.
IF(CMAKE_ANSI_CFLAGS)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")
ENDIF(CMAKE_ANSI_CFLAGS)

#-----------------------------------------------------------------------------
# Configure the standard library header wrappers based on compiler's
# capabilities and parent project's request.  Enforce 0/1 as only
# possible values for configuration into Configure.hxx.
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/kwsysPlatformCxxTests.cmake)

KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAVE_STD
  "Checking whether STL classes are in std namespace" DIRECT)

IF(KWSYS_IOS_FORCE_OLD)
  SET(KWSYS_IOS_USE_ANSI 0)
ELSE(KWSYS_IOS_FORCE_OLD)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_IOS_USE_ANSI
    "Checking whether ANSI stream headers are available" DIRECT)
ENDIF(KWSYS_IOS_FORCE_OLD)

IF(KWSYS_IOS_USE_ANSI)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_IOS_HAVE_STD
    "Checking whether ANSI streams are in std namespace" DIRECT)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_IOS_USE_SSTREAM
    "Checking whether ANSI string stream is available" DIRECT)
ELSE(KWSYS_IOS_USE_ANSI)
  SET(KWSYS_IOS_HAVE_STD 0)
  SET(KWSYS_IOS_USE_SSTREAM 0)
ENDIF(KWSYS_IOS_USE_ANSI)

IF(KWSYS_IOS_USE_SSTREAM)
  SET(KWSYS_IOS_USE_STRSTREAM_H 0)
  SET(KWSYS_IOS_USE_STRSTREA_H 0)
ELSE(KWSYS_IOS_USE_SSTREAM)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_IOS_USE_STRSTREAM_H
    "Checking whether strstream.h is available" DIRECT)
  IF(KWSYS_IOS_USE_STRSTREAM_H)
    SET(KWSYS_IOS_USE_STRSTREA_H 0)
  ELSE(KWSYS_IOS_USE_STRSTREAM_H)
    KWSYS_PLATFORM_CXX_TEST(KWSYS_IOS_USE_STRSTREA_H
      "Checking whether strstrea.h is available" DIRECT)
  ENDIF(KWSYS_IOS_USE_STRSTREAM_H)
ENDIF(KWSYS_IOS_USE_SSTREAM)

IF(KWSYS_NAMESPACE MATCHES "^kwsys$")
  SET(KWSYS_NAME_IS_KWSYS 1)
ELSE(KWSYS_NAMESPACE MATCHES "^kwsys$")
  SET(KWSYS_NAME_IS_KWSYS 0)
ENDIF(KWSYS_NAMESPACE MATCHES "^kwsys$")

# Choose default shared/static build if not specified.
IF(KWSYS_BUILD_SHARED MATCHES "^KWSYS_BUILD_SHARED$")
  SET(KWSYS_BUILD_SHARED ${BUILD_SHARED_LIBS})
ENDIF(KWSYS_BUILD_SHARED MATCHES "^KWSYS_BUILD_SHARED$")

IF(KWSYS_BUILD_SHARED)
  SET(KWSYS_BUILD_SHARED 1)
  SET(KWSYS_LIBRARY_TYPE SHARED)
ELSE(KWSYS_BUILD_SHARED)
  SET(KWSYS_BUILD_SHARED 0)
  SET(KWSYS_LIBRARY_TYPE STATIC)
ENDIF(KWSYS_BUILD_SHARED)

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
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/kwsys_stl.h.in
                 ${KWSYS_HEADER_DIR}/stl/${header}
                 @ONLY IMMEDIATE)

  # Create an install target for the header wrapper.
  IF(KWSYS_HEADER_INSTALL_DIR)
    INSTALL_FILES(${KWSYS_HEADER_INSTALL_DIR}/${KWSYS_NAMESPACE}/stl
      FILES ${KWSYS_HEADER_DIR}/stl/${header})
  ENDIF(KWSYS_HEADER_INSTALL_DIR)
ENDFOREACH(header)

#-----------------------------------------------------------------------------
# Create streams header wrappers to give standard names by which they
# may be included.
FOREACH(header iostream fstream sstream iosfwd)
  # Configure the header wrapper.
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/kwsys_ios_${header}.h.in
                 ${KWSYS_HEADER_DIR}/ios/${header}
                 @ONLY IMMEDIATE)

  # Create an install target for the header wrapper.
  IF(KWSYS_HEADER_INSTALL_DIR)
    INSTALL_FILES(${KWSYS_HEADER_INSTALL_DIR}/${KWSYS_NAMESPACE}/ios
      FILES ${KWSYS_HEADER_DIR}/ios/${header})
  ENDIF(KWSYS_HEADER_INSTALL_DIR)
ENDFOREACH(header)

#-----------------------------------------------------------------------------
# Build a list of classes and headers we need to implement the
# selected components.  Initialize with required components.
SET(KWSYS_CLASSES)
SET(KWSYS_H_FILES Configure)
SET(KWSYS_HXX_FILES Configure)

# Enforce component dependencies.
IF(KWSYS_USE_SystemTools)
  SET(KWSYS_USE_Directory 1)
ENDIF(KWSYS_USE_SystemTools)

# Add selected C++ classes.
FOREACH(c Directory RegularExpression SystemTools)
  IF(KWSYS_USE_${c})
    SET(KWSYS_CLASSES ${KWSYS_CLASSES} ${c})
  ENDIF(KWSYS_USE_${c})
ENDFOREACH(c)

# Add selected C components.
FOREACH(c Process Base64)
  IF(KWSYS_USE_${c})
    SET(KWSYS_H_FILES ${KWSYS_H_FILES} ${c})
  ENDIF(KWSYS_USE_${c})
ENDFOREACH(c)

#-----------------------------------------------------------------------------
# Build a list of sources for the library based on components that are
# included.
SET(KWSYS_SRCS)

# Add the proper sources for this platform's Process implementation.
IF(KWSYS_USE_Process)
  IF(NOT UNIX)
    # Use the Windows implementation.  We need the encoded forwarding executable.
    SET(KWSYS_SRCS ${KWSYS_SRCS} ProcessWin32.c
      ${PROJECT_BINARY_DIR}/${KWSYS_NAMESPACE}ProcessFwd9xEnc.c)
    SET_SOURCE_FILES_PROPERTIES(
      ${PROJECT_BINARY_DIR}/${KWSYS_NAMESPACE}ProcessFwd9xEnc.c
      PROPERTIES GENERATED 1)
  ELSE(NOT UNIX)
    # Use the UNIX implementation.
    SET(KWSYS_SRCS ${KWSYS_SRCS} ProcessUNIX.c)
  ENDIF(NOT UNIX)
ENDIF(KWSYS_USE_Process)

# Add sources for Base64 encoding.
IF(KWSYS_USE_Base64)
  SET(KWSYS_SRCS ${KWSYS_SRCS} Base64.c)
ENDIF(KWSYS_USE_Base64)

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
ADD_LIBRARY(${KWSYS_NAMESPACE} ${KWSYS_LIBRARY_TYPE} ${KWSYS_SRCS})

# Create an install target for the library.
IF(KWSYS_LIBRARY_INSTALL_DIR)
  INSTALL_TARGETS(${KWSYS_LIBRARY_INSTALL_DIR} ${KWSYS_NAMESPACE})
ENDIF(KWSYS_LIBRARY_INSTALL_DIR)

# For building kwsys itself, we use a macro defined on the command
# line to configure the namespace in the C and C++ source files.
ADD_DEFINITIONS("-DKWSYS_NAMESPACE=${KWSYS_NAMESPACE}")

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
  ADD_EXECUTABLE(testIOS testIOS.cxx)
  ADD_EXECUTABLE(testProcess testProcess.c)
  ADD_EXECUTABLE(test1 test1.cxx)
  TARGET_LINK_LIBRARIES(testIOS ${KWSYS_NAMESPACE})
  TARGET_LINK_LIBRARIES(testProcess ${KWSYS_NAMESPACE})
  TARGET_LINK_LIBRARIES(test1 ${KWSYS_NAMESPACE})

  IF(BUILD_TESTING)
    ADD_TEST(testProcess-1 testProcess 1)
    ADD_TEST(testProcess-2 testProcess 2)
    ADD_TEST(testProcess-3 testProcess 3)
    ADD_TEST(testProcess-4 testProcess 4)
    ADD_TEST(testProcess-5 testProcess 5)
  ENDIF(BUILD_TESTING)
ENDIF(KWSYS_STANDALONE)
