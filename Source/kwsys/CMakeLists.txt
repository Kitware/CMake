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

# NOTE: This library is intended for internal use by Kitware-driven
# projects.  In order to keep it simple no attempt will be made to
# maintain backward compatibility when changes are made to KWSys.
# When an incompatible change is made Kitware's projects that use
# KWSys will be fixed, but no notification will necessarily be sent to
# any outside mailing list and no documentation of the change will be
# written.

#-----------------------------------------------------------------------------
# If a namespace is not specified, use "kwsys" and enable testing.
# This should be the case only when kwsys is not included inside
# another project and is being tested.
IF(NOT KWSYS_NAMESPACE)
  SET(KWSYS_NAMESPACE "kwsys")
  SET(KWSYS_STANDALONE 1)
ENDIF(NOT KWSYS_NAMESPACE)

IF(KWSYS_STANDALONE OR CMake_SOURCE_DIR)
  SET(KWSYS_ENABLE_C 1)

  # Enable all components.
  SET(KWSYS_USE_Base64 1)
  SET(KWSYS_USE_Directory 1)
  SET(KWSYS_USE_DynamicLoader 1)
  SET(KWSYS_USE_Glob 1)
  SET(KWSYS_USE_Process 1)
  SET(KWSYS_USE_RegularExpression 1)
  SET(KWSYS_USE_Registry 1)
  SET(KWSYS_USE_SystemTools 1)
  SET(KWSYS_USE_CommandLineArguments 1)
  SET(KWSYS_USE_FundamentalType 1)
ENDIF(KWSYS_STANDALONE OR CMake_SOURCE_DIR)

#-----------------------------------------------------------------------------
# The project name is that of the specified namespace.
PROJECT(${KWSYS_NAMESPACE})

# Enable testing if building standalone.
IF(KWSYS_STANDALONE)
  INCLUDE(Dart)
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
# Adjust compiler flags for some platforms.
IF(NOT CMAKE_COMPILER_IS_GNUCXX)
  IF(CMAKE_SYSTEM MATCHES "OSF1-V.*")
    STRING(REGEX MATCH "-timplicit_local"
      KWSYS_CXX_FLAGS_HAVE_IMPLICIT_LOCAL "${CMAKE_CXX_FLAGS}")
    STRING(REGEX MATCH "-no_implicit_include"
      KWSYS_CXX_FLAGS_HAVE_NO_IMPLICIT_INCLUDE "${CMAKE_CXX_FLAGS}")
    IF(NOT KWSYS_CXX_FLAGS_HAVE_IMPLICIT_LOCAL)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -timplicit_local")
    ENDIF(NOT KWSYS_CXX_FLAGS_HAVE_IMPLICIT_LOCAL)
    IF(NOT KWSYS_CXX_FLAGS_HAVE_NO_IMPLICIT_INCLUDE)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -no_implicit_include")
    ENDIF(NOT KWSYS_CXX_FLAGS_HAVE_NO_IMPLICIT_INCLUDE)
  ENDIF(CMAKE_SYSTEM MATCHES "OSF1-V.*")
ENDIF(NOT CMAKE_COMPILER_IS_GNUCXX)

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

KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_CSTDDEF
  "Checking whether header cstddef is available" DIRECT)

SET(KWSYS_PLATFORM_CXX_TEST_DEFINES
  -DKWSYS_STL_HAVE_STD=${KWSYS_STL_HAVE_STD})
KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_STRING_HAVE_NEQ_CHAR
  "Checking whether stl string has operator!= for char*" DIRECT)
KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS_ITERATOR_TRAITS
  "Checking whether stl has iterator_traits" DIRECT)
IF(KWSYS_STL_HAS_ITERATOR_TRAITS)
  SET(KWSYS_STL_HAS_ITERATOR_CATEGORY 0)
  SET(KWSYS_STL_HAS___ITERATOR_CATEGORY 0)
ELSE(KWSYS_STL_HAS_ITERATOR_TRAITS)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS_ITERATOR_CATEGORY
    "Checking whether stl has old iterator_category" DIRECT)
  IF(KWSYS_STL_HAS_ITERATOR_CATEGORY)
    SET(KWSYS_STL_HAS___ITERATOR_CATEGORY 0)
  ELSE(KWSYS_STL_HAS_ITERATOR_CATEGORY)
    KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS___ITERATOR_CATEGORY
      "Checking whether stl has internal __iterator_category" DIRECT)
  ENDIF(KWSYS_STL_HAS_ITERATOR_CATEGORY)
ENDIF(KWSYS_STL_HAS_ITERATOR_TRAITS)
KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS_ALLOCATOR_TEMPLATE
  "Checking whether stl has standard template allocator" DIRECT)
IF(KWSYS_STL_HAS_ALLOCATOR_TEMPLATE)
  SET(KWSYS_STL_HAS_ALLOCATOR_NONTEMPLATE 0)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS_ALLOCATOR_REBIND
    "Checking for rebind member of stl allocator" DIRECT)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS_ALLOCATOR_MAX_SIZE_ARGUMENT
    "Checking for non-standard argument to stl allocator<>::max_size" DIRECT)
ELSE(KWSYS_STL_HAS_ALLOCATOR_TEMPLATE)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS_ALLOCATOR_NONTEMPLATE
    "Checking whether stl has old non-template allocator" DIRECT)
  SET(KWSYS_STL_HAS_ALLOCATOR_REBIND 0)
  SET(KWSYS_STL_HAS_ALLOCATOR_MAX_SIZE_ARGUMENT 0)
ENDIF(KWSYS_STL_HAS_ALLOCATOR_TEMPLATE)
KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS_ALLOCATOR_OBJECTS
  "Checking whether stl containers support allocator objects." DIRECT)
IF(KWSYS_IOS_USE_ANSI)
  # ANSI streams always have string operators.
  SET(KWSYS_STL_STRING_HAVE_OSTREAM 1)
  SET(KWSYS_STL_STRING_HAVE_ISTREAM 1)
ELSE(KWSYS_IOS_USE_ANSI)
  # There may not be string operators for old streams.
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_STRING_HAVE_OSTREAM
    "Checking whether stl string has ostream operator<<" DIRECT)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_STRING_HAVE_ISTREAM
    "Checking whether stl string has istream operator>>" DIRECT)
ENDIF(KWSYS_IOS_USE_ANSI)
SET(KWSYS_PLATFORM_CXX_TEST_DEFINES)

KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_NULL_TEMPLATE_ARGS
  "Checking whether \"<>\" is needed for template friends" INVERT)
KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_MEMBER_TEMPLATES
  "Checking for member template support" DIRECT)
KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_FULL_SPECIALIZATION
  "Checking for standard template specialization syntax" DIRECT)
KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_ARGUMENT_DEPENDENT_LOOKUP
  "Checking whether argument dependent lookup is supported" DIRECT)

IF(UNIX)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STAT_HAS_ST_MTIM
    "Checking whether struct stat has st_mtim member" DIRECT)
ENDIF(UNIX)

IF(KWSYS_USE_FundamentalType)
  # Determine type sizes.
  INCLUDE(CheckTypeSize)
  CHECK_TYPE_SIZE("char"      KWSYS_SIZEOF_CHAR)
  CHECK_TYPE_SIZE("short"     KWSYS_SIZEOF_SHORT)
  CHECK_TYPE_SIZE("int"       KWSYS_SIZEOF_INT)
  CHECK_TYPE_SIZE("long"      KWSYS_SIZEOF_LONG)
  CHECK_TYPE_SIZE("long long" KWSYS_SIZEOF_LONG_LONG)
  CHECK_TYPE_SIZE("__int64"   KWSYS_SIZEOF___INT64)
  IF(NOT KWSYS_SIZEOF_LONG_LONG)
    SET(KWSYS_SIZEOF_LONG_LONG 0)
  ENDIF(NOT KWSYS_SIZEOF_LONG_LONG)
  IF(NOT KWSYS_SIZEOF___INT64)
    SET(KWSYS_SIZEOF___INT64 0)
  ENDIF(NOT KWSYS_SIZEOF___INT64)

  # Check uniqueness of types.
  IF(KWSYS_SIZEOF___INT64)
    KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_SAME_LONG_AND___INT64
      "Checking whether long and __int64 are the same type" DIRECT)
    IF(KWSYS_SIZEOF_LONG_LONG)
      KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_SAME_LONG_LONG_AND___INT64
        "Checking whether long long and __int64 are the same type" DIRECT)
    ENDIF(KWSYS_SIZEOF_LONG_LONG)
  ENDIF(KWSYS_SIZEOF___INT64)

  # Enable the "long long" type if it is available.  It is standard in
  # C99 and C++03 but not in earlier standards.
  IF(KWSYS_SIZEOF_LONG_LONG)
    SET(KWSYS_USE_LONG_LONG 1)
  ELSE(KWSYS_SIZEOF_LONG_LONG)
    SET(KWSYS_USE_LONG_LONG 0)
  ENDIF(KWSYS_SIZEOF_LONG_LONG)

  # Enable the "__int64" type if it is available and unique.  It is not
  # standard.
  SET(KWSYS_USE___INT64 0)
  IF(KWSYS_SIZEOF___INT64)
    IF(NOT KWSYS_CXX_SAME_LONG_AND___INT64)
      IF(NOT KWSYS_CXX_SAME_LONG_LONG_AND___INT64)
        SET(KWSYS_USE___INT64 1)
      ENDIF(NOT KWSYS_CXX_SAME_LONG_LONG_AND___INT64)
    ENDIF(NOT KWSYS_CXX_SAME_LONG_AND___INT64)
  ENDIF(KWSYS_SIZEOF___INT64)
  IF(KWSYS_USE___INT64)
    KWSYS_PLATFORM_CXX_TEST(KWSYS_CAN_CONVERT_UI64_TO_DOUBLE
      "Checking whether unsigned __int64 can convert to double" DIRECT)
  ELSE(KWSYS_USE___INT64)
    SET(KWSYS_CAN_CONVERT_UI64_TO_DOUBLE 1)
  ENDIF(KWSYS_USE___INT64)

  # Check signedness of "char" type.
  IF("KWSYS_CHAR_IS_SIGNED" MATCHES "^KWSYS_CHAR_IS_SIGNED$")
    MESSAGE(STATUS "Checking signedness of char")
    TRY_RUN(KWSYS_CHAR_IS_SIGNED KWSYS_CHAR_IS_SIGNED_COMPILED
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/kwsysPlatformCxxTests.cxx
      COMPILE_DEFINITIONS -DTEST_KWSYS_CHAR_IS_SIGNED)
    IF(KWSYS_CHAR_IS_SIGNED_COMPILED)
      IF(KWSYS_CHAR_IS_SIGNED)
        MESSAGE(STATUS "Checking signedness of char -- signed")
        SET(KWSYS_CHAR_IS_SIGNED 1 CACHE INTERNAL "Whether char is signed.")
      ELSE(KWSYS_CHAR_IS_SIGNED)
        MESSAGE(STATUS "Checking signedness of char -- unsigned")
        SET(KWSYS_CHAR_IS_SIGNED 0 CACHE INTERNAL "Whether char is signed.")
      ENDIF(KWSYS_CHAR_IS_SIGNED)
    ELSE(KWSYS_CHAR_IS_SIGNED_COMPILED)
      MESSAGE(FATAL_ERROR "Checking signedness of char -- failed")
    ENDIF(KWSYS_CHAR_IS_SIGNED_COMPILED)
  ENDIF("KWSYS_CHAR_IS_SIGNED" MATCHES "^KWSYS_CHAR_IS_SIGNED$")
ENDIF(KWSYS_USE_FundamentalType)

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
INCLUDE_DIRECTORIES(${KWSYS_HEADER_ROOT})

#-----------------------------------------------------------------------------
# Create STL header wrappers to block warnings in the STL headers and
# give standard names by which they may be included.
SET(KWSYS_STL_HEADER_EXTRA_string 1)
FOREACH(header algorithm deque iterator list map numeric queue set stack string
               utility vector memory functional)
  # Configure the header wrapper.
  SET(KWSYS_STL_HEADER "${header}")
  IF(KWSYS_STL_HEADER_EXTRA_${header})
    SET(KWSYS_STL_HEADER_EXTRA
      "#define ${KWSYS_NAMESPACE}_stl_${header}_including_hxx\n# include <${KWSYS_NAMESPACE}/stl/${header}.hxx>\n#undef ${KWSYS_NAMESPACE}_stl_${header}_including_hxx\n")
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/kwsys_stl_${header}.hxx.in
                   ${KWSYS_HEADER_DIR}/stl/${header}.hxx
                   @ONLY IMMEDIATE)
    IF(KWSYS_HEADER_INSTALL_DIR)
      INSTALL_FILES(${KWSYS_HEADER_INSTALL_DIR}/${KWSYS_NAMESPACE}/stl
        FILES ${KWSYS_HEADER_DIR}/stl/${header}.hxx)
    ENDIF(KWSYS_HEADER_INSTALL_DIR)
  ELSE(KWSYS_STL_HEADER_EXTRA_${header})
    SET(KWSYS_STL_HEADER_EXTRA "")
  ENDIF(KWSYS_STL_HEADER_EXTRA_${header})
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/kwsys_stl.hxx.in
                 ${KWSYS_HEADER_DIR}/stl/${header}
                 @ONLY IMMEDIATE)

  # Create an install target for the header wrapper.
  IF(KWSYS_HEADER_INSTALL_DIR)
    INSTALL_FILES(${KWSYS_HEADER_INSTALL_DIR}/${KWSYS_NAMESPACE}/stl
      FILES ${KWSYS_HEADER_DIR}/stl/${header})
  ENDIF(KWSYS_HEADER_INSTALL_DIR)
ENDFOREACH(header)

# Provide cstddef header.
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/kwsys_cstddef.hxx.in
               ${KWSYS_HEADER_DIR}/cstddef
               @ONLY IMMEDIATE)

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
SET(KWSYS_H_FILES Configure SharedForward)
SET(KWSYS_HXX_FILES Configure String
  hashtable hash_fun hash_map hash_set
  auto_ptr
  )

# Enforce component dependencies.
IF(KWSYS_USE_SystemTools)
  SET(KWSYS_USE_Directory 1)
ENDIF(KWSYS_USE_SystemTools)
IF(KWSYS_USE_Glob)
  SET(KWSYS_USE_Directory 1)
  SET(KWSYS_USE_SystemTools 1)
  SET(KWSYS_USE_RegularExpression 1)
ENDIF(KWSYS_USE_Glob)

# Add selected C++ classes.
SET(cppclasses Directory DynamicLoader Glob RegularExpression SystemTools CommandLineArguments Registry)
FOREACH(cpp ${cppclasses})
  IF(KWSYS_USE_${cpp})
    SET(KWSYS_CLASSES ${KWSYS_CLASSES} ${cpp})
  ENDIF(KWSYS_USE_${cpp})
ENDFOREACH(cpp)

# Add selected C components.
FOREACH(c Process Base64 FundamentalType)
  IF(KWSYS_USE_${c})
    SET(KWSYS_H_FILES ${KWSYS_H_FILES} ${c})
  ENDIF(KWSYS_USE_${c})
ENDFOREACH(c)

#-----------------------------------------------------------------------------
# Build a list of sources for the library based on components that are
# included.
SET(KWSYS_C_SRCS)
SET(KWSYS_CXX_SRCS)

# Add the proper sources for this platform's Process implementation.
IF(KWSYS_USE_Process)
  IF(NOT UNIX)
    # Use the Windows implementation.  We need the encoded forwarding executable.
    SET(KWSYS_C_SRCS ${KWSYS_C_SRCS} ProcessWin32.c
      ${PROJECT_BINARY_DIR}/${KWSYS_NAMESPACE}ProcessFwd9xEnc.c)
    SET_SOURCE_FILES_PROPERTIES(
      ${PROJECT_BINARY_DIR}/${KWSYS_NAMESPACE}ProcessFwd9xEnc.c
      PROPERTIES GENERATED 1)
  ELSE(NOT UNIX)
    # Use the UNIX implementation.
    SET(KWSYS_C_SRCS ${KWSYS_C_SRCS} ProcessUNIX.c)
  ENDIF(NOT UNIX)
ENDIF(KWSYS_USE_Process)

# Add sources for Base64 encoding.
IF(KWSYS_USE_Base64)
  SET(KWSYS_C_SRCS ${KWSYS_C_SRCS} Base64.c)
ENDIF(KWSYS_USE_Base64)

# Configure headers of C++ classes and construct the list of sources.
FOREACH(c ${KWSYS_CLASSES})
  # Add this source to the list of source files for the library.
  SET(KWSYS_CXX_SRCS ${KWSYS_CXX_SRCS} ${c}.cxx)

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
IF(KWSYS_C_SRCS OR KWSYS_CXX_SRCS)
  ADD_LIBRARY(${KWSYS_NAMESPACE} ${KWSYS_LIBRARY_TYPE}
    ${KWSYS_C_SRCS} ${KWSYS_CXX_SRCS})

  # Apply user-defined target properties to the library.
  IF(KWSYS_PROPERTIES_CXX)
    SET_TARGET_PROPERTIES(${KWSYS_NAMESPACE} PROPERTIES
      ${KWSYS_PROPERTIES_CXX}
      )
  ENDIF(KWSYS_PROPERTIES_CXX)

  # Create an install target for the library.
  IF(KWSYS_LIBRARY_INSTALL_DIR)
    INSTALL_TARGETS(${KWSYS_LIBRARY_INSTALL_DIR} ${KWSYS_NAMESPACE})
  ENDIF(KWSYS_LIBRARY_INSTALL_DIR)
ENDIF(KWSYS_C_SRCS OR KWSYS_CXX_SRCS)

# Add a C-only library if requested.
IF(KWSYS_ENABLE_C AND KWSYS_C_SRCS)
  ADD_LIBRARY(${KWSYS_NAMESPACE}_c ${KWSYS_LIBRARY_TYPE} ${KWSYS_C_SRCS})

  # Apply user-defined target properties to the library.
  IF(KWSYS_PROPERTIES_C)
    SET_TARGET_PROPERTIES(${KWSYS_NAMESPACE} PROPERTIES
      ${KWSYS_PROPERTIES_C}
      )
  ENDIF(KWSYS_PROPERTIES_C)

  # Create an install target for the library.
  IF(KWSYS_LIBRARY_INSTALL_DIR)
    INSTALL_TARGETS(${KWSYS_LIBRARY_INSTALL_DIR} ${KWSYS_NAMESPACE}_c)
  ENDIF(KWSYS_LIBRARY_INSTALL_DIR)
ENDIF(KWSYS_ENABLE_C AND KWSYS_C_SRCS)

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
IF(KWSYS_STANDALONE OR CMake_SOURCE_DIR)
  IF(BUILD_TESTING)
    # C++ tests
    IF(KWSYS_USE_DynamicLoader)
      SET(EXTRA_TESTS
        testDynamicLoader
        )
      # If kwsys contains the DynamicLoader, need extra library
      ADD_LIBRARY(testDynload MODULE testDynload.c)
      IF(UNIX)
        TARGET_LINK_LIBRARIES(${KWSYS_NAMESPACE} ${CMAKE_DL_LIBS})
      ENDIF(UNIX)
    ENDIF(KWSYS_USE_DynamicLoader)
    FOREACH(test
        testSystemTools
        testIOS
        testHashSTL
        testCommandLineArguments
        testRegistry
        ${EXTRA_TESTS}
        )
      ADD_EXECUTABLE(${test} ${test}.cxx)
      TARGET_LINK_LIBRARIES(${test} ${KWSYS_NAMESPACE})
    ENDFOREACH(test)
    # C tests
    FOREACH(test
        testProcess
        testFail
        )
      ADD_EXECUTABLE(${test} ${test}.c)
      TARGET_LINK_LIBRARIES(${test} ${KWSYS_NAMESPACE}_c)
    ENDFOREACH(test)

    SET(EXEC_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    IF(EXECUTABLE_OUTPUT_PATH)
      SET(EXEC_DIR "${EXECUTABLE_OUTPUT_PATH}")
    ENDIF(EXECUTABLE_OUTPUT_PATH)


    SET(TEST_SYSTEMTOOLS_BIN_FILE
      "${CMAKE_CURRENT_SOURCE_DIR}/testSystemTools.bin")
    SET(TEST_SYSTEMTOOLS_SRC_FILE
      "${CMAKE_CURRENT_SOURCE_DIR}/testSystemTools.cxx")
    CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/testSystemTools.h.in
      ${PROJECT_BINARY_DIR}/testSystemTools.h)
    INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

    IF(CTEST_TEST_KWSYS)
      CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/ExtraTest.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ExtraTest.cmake")
      SET_DIRECTORY_PROPERTIES(PROPERTIES TEST_INCLUDE_FILE "${CMAKE_CURRENT_BINARY_DIR}/ExtraTest.cmake")
    ENDIF(CTEST_TEST_KWSYS)
    ADD_TEST(kwsys.testSystemTools ${EXEC_DIR}/testSystemTools)
    IF(KWSYS_USE_DynamicLoader)
      ADD_TEST(kwsys.testDynamicLoader ${EXEC_DIR}/testDynamicLoader)
    ENDIF(KWSYS_USE_DynamicLoader)
    ADD_TEST(kwsys.testProcess-1 ${EXEC_DIR}/testProcess 1)
    ADD_TEST(kwsys.testProcess-2 ${EXEC_DIR}/testProcess 2)
    ADD_TEST(kwsys.testProcess-3 ${EXEC_DIR}/testProcess 3)
    ADD_TEST(kwsys.testProcess-4 ${EXEC_DIR}/testProcess 4)
    ADD_TEST(kwsys.testProcess-5 ${EXEC_DIR}/testProcess 5)
    ADD_TEST(kwsys.testProcess-6 ${EXEC_DIR}/testProcess 6)
    ADD_TEST(kwsys.testProcess-7 ${EXEC_DIR}/testProcess 7)
    ADD_TEST(kwsys.testHashSTL ${EXEC_DIR}/testHashSTL)
    ADD_TEST(kwsys.testRegistry ${EXEC_DIR}/testRegistry)
    IF(COMMAND SET_TESTS_PROPERTIES AND COMMAND GET_TEST_PROPERTY AND KWSYS_STANDALONE)
      ADD_TEST(kwsys.testFail ${EXEC_DIR}/testFail)
      # We expect test to fail
      SET_TESTS_PROPERTIES(kwsys.testFail PROPERTIES WILL_FAIL ON)
      GET_TEST_PROPERTY(kwsys.testFail WILL_FAIL wfv)
      SET_TESTS_PROPERTIES(kwsys.testRegistry PROPERTIES FAIL_REGULAR_EXPRESSION "ERROR;FAIL;Test failed")
      SET_TESTS_PROPERTIES(kwsys.testRegistry PROPERTIES PASS_REGULAR_EXPRESSION "Test passed")
      MESSAGE(STATUS "GET_TEST_PROPERTY returned: ${wfv}")
    ENDIF(COMMAND SET_TESTS_PROPERTIES AND COMMAND GET_TEST_PROPERTY AND KWSYS_STANDALONE)
  ENDIF(BUILD_TESTING)
ENDIF(KWSYS_STANDALONE OR CMake_SOURCE_DIR)
