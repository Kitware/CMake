#=============================================================================
# KWSys - Kitware System Library
# Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
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
#  KWSYS_IOS_FORCE_OLD = Force use of old non-ANSI C++ streams even if
#                        new streams are available.  This may be used
#                        by projects that cannot configure their
#                        streams library.
#    Example:
#
#      SET(KWSYS_IOS_FORCE_OLD 1)
#
#
# Optional settings to setup install rules work in one of two ways.
# The modern way utilizes the CMake 2.4 INSTALL command.  Settings
# for this mode are as follows:
#
#  KWSYS_INSTALL_BIN_DIR     = The installation target directories into
#  KWSYS_INSTALL_LIB_DIR       which the libraries and headers from
#  KWSYS_INSTALL_INCLUDE_DIR   kwsys should be installed by a "make install".
#                              The values should be specified relative to
#                              the installation prefix and NOT start with '/'.
#  KWSYS_INSTALL_DOC_DIR     = The installation target directory for documentation
#                              such as copyright information.
#
#  KWSYS_INSTALL_COMPONENT_NAME_RUNTIME     = Name of runtime and development
#  KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT   installation components.
#                                             If not given the install rules
#                                             will not be in any component.
#
#  KWSYS_INSTALL_EXPORT_NAME = The EXPORT option value for install(TARGETS) calls.
#
#    Example:
#
#      SET(KWSYS_INSTALL_BIN_DIR bin)
#      SET(KWSYS_INSTALL_LIB_DIR lib)
#      SET(KWSYS_INSTALL_INCLUDE_DIR include)
#      SET(KWSYS_INSTALL_COMPONENT_NAME_RUNTIME Runtime)
#      SET(KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT Development)
#
# The old way uses the original CMake INSTALL_* commands.  Settings
# for this mode are as follows:
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
# The modern way will be used whenever the INSTALL command is
# available.  If the settings are not available the old names will be
# used to construct them.  The old way will be used whenever the
# INSTALL command is not available.  If the settings are not available
# the new names will be used to construct them.

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

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.5 FATAL_ERROR)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

# Allow empty endif() and such with CMake 2.4.
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)

#-----------------------------------------------------------------------------
# If a namespace is not specified, use "kwsys" and enable testing.
# This should be the case only when kwsys is not included inside
# another project and is being tested.
IF(NOT KWSYS_NAMESPACE)
  SET(KWSYS_NAMESPACE "kwsys")
  SET(KWSYS_STANDALONE 1)
ENDIF(NOT KWSYS_NAMESPACE)

#-----------------------------------------------------------------------------
# The project name is that of the specified namespace.
PROJECT(${KWSYS_NAMESPACE})

# Some properties we set only with CMake 2.6 and above.
IF(COMMAND SET_PROPERTY)
  MACRO(KWSYS_SET_PROPERTY)
    SET_PROPERTY(${ARGV})
  ENDMACRO(KWSYS_SET_PROPERTY)
ELSE(COMMAND SET_PROPERTY)
  MACRO(KWSYS_SET_PROPERTY)
  ENDMACRO(KWSYS_SET_PROPERTY)
ENDIF(COMMAND SET_PROPERTY)

# Tell CMake how to follow dependencies of sources in this directory.
IF(COMMAND SET_PROPERTY)
  SET_PROPERTY(DIRECTORY
    PROPERTY IMPLICIT_DEPENDS_INCLUDE_TRANSFORM
    "KWSYS_HEADER(%)=<${KWSYS_NAMESPACE}/%>"
    )
ENDIF(COMMAND SET_PROPERTY)

# Select library components.
IF(KWSYS_STANDALONE OR CMake_SOURCE_DIR)
  SET(KWSYS_ENABLE_C 1)
  # Enable all components.
  SET(KWSYS_USE_Base64 1)
  SET(KWSYS_USE_Directory 1)
  SET(KWSYS_USE_DynamicLoader 1)
  SET(KWSYS_USE_Glob 1)
  SET(KWSYS_USE_MD5 1)
  SET(KWSYS_USE_Process 1)
  SET(KWSYS_USE_RegularExpression 1)
  SET(KWSYS_USE_Registry 1)
  SET(KWSYS_USE_System 1)
  SET(KWSYS_USE_SystemTools 1)
  SET(KWSYS_USE_CommandLineArguments 1)
  SET(KWSYS_USE_FundamentalType 1)
  SET(KWSYS_USE_Terminal 1)
  SET(KWSYS_USE_IOStream 1)
  SET(KWSYS_USE_DateStamp 1)
  SET(KWSYS_USE_String 1)
  SET(KWSYS_USE_SystemInformation 1)
  SET(KWSYS_USE_CPU 1)
ENDIF(KWSYS_STANDALONE OR CMake_SOURCE_DIR)

# Enforce component dependencies.
IF(KWSYS_USE_SystemTools)
  SET(KWSYS_USE_Directory 1)
ENDIF(KWSYS_USE_SystemTools)
IF(KWSYS_USE_Glob)
  SET(KWSYS_USE_Directory 1)
  SET(KWSYS_USE_SystemTools 1)
  SET(KWSYS_USE_RegularExpression 1)
ENDIF(KWSYS_USE_Glob)
IF(KWSYS_USE_Process)
  SET(KWSYS_USE_System 1)
ENDIF(KWSYS_USE_Process)
IF(KWSYS_USE_SystemInformation)
  SET(KWSYS_USE_FundamentalType 1)
  SET(KWSYS_USE_Process 1)
ENDIF(KWSYS_USE_SystemInformation)

# Setup the large file support default.
IF(KWSYS_LFS_DISABLE)
  SET(KWSYS_LFS_REQUESTED 0)
ELSE(KWSYS_LFS_DISABLE)
  SET(KWSYS_LFS_REQUESTED 1)
ENDIF(KWSYS_LFS_DISABLE)

# Enable testing if building standalone.
IF(KWSYS_STANDALONE)
  INCLUDE(Dart)
  MARK_AS_ADVANCED(BUILD_TESTING DART_ROOT TCL_TCLSH)
  IF(BUILD_TESTING)
    ENABLE_TESTING()
  ENDIF(BUILD_TESTING)
ENDIF(KWSYS_STANDALONE)

# Include helper macros.
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/kwsysPlatformTests.cmake)
INCLUDE(CheckTypeSize)

# Do full dependency headers.
INCLUDE_REGULAR_EXPRESSION("^.*$")

# Choose which kind of install commands to use.
IF(COMMAND INSTALL)
  # Use new KWSYS_INSTALL_*_DIR variable names to control installation.
  # Take defaults from the old names.  Note that there was no old name
  # for the bin dir, so we take the old lib dir name so DLLs will be
  # installed in a compatible way for old code.
  IF(NOT KWSYS_INSTALL_INCLUDE_DIR)
    STRING(REGEX REPLACE "^/" "" KWSYS_INSTALL_INCLUDE_DIR
      "${KWSYS_HEADER_INSTALL_DIR}")
  ENDIF(NOT KWSYS_INSTALL_INCLUDE_DIR)
  IF(NOT KWSYS_INSTALL_LIB_DIR)
    STRING(REGEX REPLACE "^/" "" KWSYS_INSTALL_LIB_DIR
      "${KWSYS_LIBRARY_INSTALL_DIR}")
  ENDIF(NOT KWSYS_INSTALL_LIB_DIR)
  IF(NOT KWSYS_INSTALL_BIN_DIR)
    STRING(REGEX REPLACE "^/" "" KWSYS_INSTALL_BIN_DIR
      "${KWSYS_LIBRARY_INSTALL_DIR}")
  ENDIF(NOT KWSYS_INSTALL_BIN_DIR)

  # Setup header install rules.
  SET(KWSYS_INSTALL_INCLUDE_OPTIONS)
  IF(KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT)
    SET(KWSYS_INSTALL_INCLUDE_OPTIONS ${KWSYS_INSTALL_INCLUDE_OPTIONS}
      COMPONENT ${KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT}
      )
  ENDIF(KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT)

  # Setup library install rules.
  SET(KWSYS_INSTALL_LIBRARY_RULE)
  IF(KWSYS_INSTALL_LIB_DIR)
    IF(KWSYS_INSTALL_EXPORT_NAME)
      LIST(APPEND KWSYS_INSTALL_LIBRARY_RULE EXPORT ${KWSYS_INSTALL_EXPORT_NAME})
    ENDIF()
    # Install the shared library to the lib directory.
    SET(KWSYS_INSTALL_LIBRARY_RULE ${KWSYS_INSTALL_LIBRARY_RULE}
      LIBRARY DESTINATION ${KWSYS_INSTALL_LIB_DIR}
      )
    # Assign the shared library to the runtime component.
    IF(KWSYS_INSTALL_COMPONENT_NAME_RUNTIME)
      SET(KWSYS_INSTALL_LIBRARY_RULE ${KWSYS_INSTALL_LIBRARY_RULE}
        COMPONENT ${KWSYS_INSTALL_COMPONENT_NAME_RUNTIME}
        )
    ENDIF(KWSYS_INSTALL_COMPONENT_NAME_RUNTIME)

    # Install the archive to the lib directory.
    SET(KWSYS_INSTALL_LIBRARY_RULE ${KWSYS_INSTALL_LIBRARY_RULE}
      ARCHIVE DESTINATION ${KWSYS_INSTALL_LIB_DIR}
      )
    # Assign the archive to the development component.
    IF(KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT)
      SET(KWSYS_INSTALL_LIBRARY_RULE ${KWSYS_INSTALL_LIBRARY_RULE}
        COMPONENT ${KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT}
        )
    ENDIF(KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT)
  ENDIF(KWSYS_INSTALL_LIB_DIR)
  IF(KWSYS_INSTALL_BIN_DIR)
    # Install the runtime library to the bin directory.
    SET(KWSYS_INSTALL_LIBRARY_RULE ${KWSYS_INSTALL_LIBRARY_RULE}
      RUNTIME DESTINATION ${KWSYS_INSTALL_BIN_DIR}
      )
    # Assign the runtime library to the runtime component.
    IF(KWSYS_INSTALL_COMPONENT_NAME_RUNTIME)
      SET(KWSYS_INSTALL_LIBRARY_RULE ${KWSYS_INSTALL_LIBRARY_RULE}
        COMPONENT ${KWSYS_INSTALL_COMPONENT_NAME_RUNTIME}
        )
    ENDIF(KWSYS_INSTALL_COMPONENT_NAME_RUNTIME)
  ENDIF(KWSYS_INSTALL_BIN_DIR)

  # Do not support old KWSYS_*_INSTALL_DIR variable names.
  SET(KWSYS_HEADER_INSTALL_DIR)
  SET(KWSYS_LIBRARY_INSTALL_DIR)

ELSE(COMMAND INSTALL)
  # Use old KWSYS_*_INSTALL_DIR variable names.
  # Take defaults from the new names.
  IF(KWSYS_INSTALL_LIB_DIR)
    IF(NOT KWSYS_LIBRARY_INSTALL_DIR)
      SET(KWSYS_LIBRARY_INSTALL_DIR "/${KWSYS_INSTALL_LIB_DIR}")
    ENDIF(NOT KWSYS_LIBRARY_INSTALL_DIR)
  ENDIF(KWSYS_INSTALL_LIB_DIR)
  IF(KWSYS_INSTALL_INCLUDE_DIR)
    IF(NOT KWSYS_HEADER_INSTALL_DIR)
      SET(KWSYS_HEADER_INSTALL_DIR "/${KWSYS_INSTALL_INCLUDE_DIR}")
    ENDIF(NOT KWSYS_HEADER_INSTALL_DIR)
  ENDIF(KWSYS_INSTALL_INCLUDE_DIR)

  # Do not support new KWSYS_INSTALL_*_DIR variable names.
  SET(KWSYS_INSTALL_BIN_DIR)
  SET(KWSYS_INSTALL_INCLUDE_DIR)
  SET(KWSYS_INSTALL_LIB_DIR)
ENDIF(COMMAND INSTALL)

# Generated source files will need this header.
STRING(COMPARE EQUAL "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}"
  KWSYS_IN_SOURCE_BUILD)
IF(NOT KWSYS_IN_SOURCE_BUILD)
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/kwsysPrivate.h
    ${PROJECT_BINARY_DIR}/kwsysPrivate.h COPY_ONLY IMMEDIATE)
ENDIF(NOT KWSYS_IN_SOURCE_BUILD)

# Select plugin module file name convention.
IF(NOT KWSYS_DynamicLoader_PREFIX)
  SET(KWSYS_DynamicLoader_PREFIX ${CMAKE_SHARED_MODULE_PREFIX})
ENDIF()
IF(NOT KWSYS_DynamicLoader_SUFFIX)
  SET(KWSYS_DynamicLoader_SUFFIX ${CMAKE_SHARED_MODULE_SUFFIX})
ENDIF()

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
  IF(CMAKE_SYSTEM MATCHES "HP-UX")
    SET(KWSYS_PLATFORM_CXX_TEST_EXTRA_FLAGS "+p")
  ENDIF(CMAKE_SYSTEM MATCHES "HP-UX")
ENDIF(NOT CMAKE_COMPILER_IS_GNUCXX)

#-----------------------------------------------------------------------------
# Configure Large File Support.
KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_CSTDIO
  "Checking whether header cstdio is available" DIRECT)
SET(KWSYS_LFS_AVAILABLE 0)
IF(KWSYS_LFS_REQUESTED)
  # Large File Support is requested.
  SET(KWSYS_LFS_REQUESTED 1)

  # Check for large file support.
  SET(KWSYS_PLATFORM_CXX_TEST_DEFINES
    -DKWSYS_CXX_HAS_CSTDIO=${KWSYS_CXX_HAS_CSTDIO})
  KWSYS_PLATFORM_CXX_TEST_RUN(KWSYS_LFS_WORKS
    "Checking for Large File Support" DIRECT)
  SET(KWSYS_PLATFORM_CXX_TEST_DEFINES)

  IF(KWSYS_LFS_WORKS)
    SET(KWSYS_LFS_AVAILABLE 1)
  ENDIF(KWSYS_LFS_WORKS)
ELSE(KWSYS_LFS_REQUESTED)
  # Large File Support is not requested.
  SET(KWSYS_LFS_REQUESTED 0)
ENDIF(KWSYS_LFS_REQUESTED)

#-----------------------------------------------------------------------------
# Configure the standard library header wrappers based on compiler's
# capabilities and parent project's request.  Enforce 0/1 as only
# possible values for configuration into Configure.hxx.

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
IF(KWSYS_IOS_USE_ANSI AND NOT WATCOM)
  # ANSI streams always have string operators.
  SET(KWSYS_STL_STRING_HAVE_OSTREAM 1)
  SET(KWSYS_STL_STRING_HAVE_ISTREAM 1)
ELSE(KWSYS_IOS_USE_ANSI AND NOT WATCOM)
  # There may not be string operators for old streams.
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_STRING_HAVE_OSTREAM
    "Checking whether stl string has ostream operator<<" DIRECT)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_STRING_HAVE_ISTREAM
    "Checking whether stl string has istream operator>>" DIRECT)
ENDIF(KWSYS_IOS_USE_ANSI AND NOT WATCOM)
SET(KWSYS_PLATFORM_CXX_TEST_DEFINES
  -DKWSYS_IOS_USE_ANSI=${KWSYS_IOS_USE_ANSI}
  -DKWSYS_IOS_HAVE_STD=${KWSYS_IOS_HAVE_STD})
KWSYS_PLATFORM_CXX_TEST(KWSYS_IOS_HAVE_BINARY
  "Checking whether ios has binary openmode" DIRECT)
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
  # Look for type size helper macros.
  KWSYS_PLATFORM_INFO_TEST(C KWSYS_C_TYPE_MACROS
    "Checking for C type size macros")
  SET(macro_regex ".*INFO:macro\\[([^]]*)\\].*")
  FOREACH(info ${KWSYS_C_TYPE_MACROS})
    IF("${info}" MATCHES "${macro_regex}")
      STRING(REGEX REPLACE "${macro_regex}" "\\1" macro "${info}")
      SET(KWSYS_C_HAS_MACRO_${macro} 1)
    ENDIF()
  ENDFOREACH()

  # Determine type sizes at preprocessing time if possible, and
  # otherwise fall back to a try-compile.
  SET(KWSYS_C_TYPE_NAME_CHAR      "char")
  SET(KWSYS_C_TYPE_NAME_SHORT     "short")
  SET(KWSYS_C_TYPE_NAME_INT       "int")
  SET(KWSYS_C_TYPE_NAME_LONG      "long")
  SET(KWSYS_C_TYPE_NAME_LONG_LONG "long long")
  SET(KWSYS_C_TYPE_NAME___INT64   "__int64")
  FOREACH(type CHAR SHORT INT LONG LONG_LONG __INT64)
    IF(KWSYS_C_HAS_MACRO___SIZEOF_${type}__)
      # Use __SIZEOF_${type}__ macro.
      SET(KWSYS_SIZEOF_${type} TRUE)
      SET(KWSYS_C_CODE_SIZEOF_${type} "#define ${KWSYS_NAMESPACE}_SIZEOF_${type} __SIZEOF_${type}__")
    ELSEIF(KWSYS_C_HAS_MACRO___${type}_MAX__)
      # Use __${type}_MAX__ macro.
      SET(KWSYS_SIZEOF_${type} TRUE)
      SET(KWSYS_C_CODE_SIZEOF_${type} "#if __${type}_MAX__ == 0x7f
# define ${KWSYS_NAMESPACE}_SIZEOF_${type} 1
#elif __${type}_MAX__ == 0x7fff
# define ${KWSYS_NAMESPACE}_SIZEOF_${type} 2
#elif __${type}_MAX__ == 0x7fffffff
# define ${KWSYS_NAMESPACE}_SIZEOF_${type} 4
#elif __${type}_MAX__>>32 == 0x7fffffff
# define ${KWSYS_NAMESPACE}_SIZEOF_${type} 8
#else
# error \"Cannot determine sizeof(${KWSYS_C_TYPE_NAME_${type}}).\"
#endif")
    ELSE()
      # Configure a hard-coded type size.
      CHECK_TYPE_SIZE("${KWSYS_C_TYPE_NAME_${type}}" KWSYS_SIZEOF_${type})
      IF(NOT KWSYS_SIZEOF_${type})
        SET(KWSYS_SIZEOF_${type} 0)
      ENDIF()
      SET(KWSYS_C_CODE_SIZEOF_${type}
        "#define ${KWSYS_NAMESPACE}_SIZEOF_${type} ${KWSYS_SIZEOF_${type}}")
    ENDIF()
  ENDFOREACH()

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
  KWSYS_PLATFORM_CXX_TEST_RUN(KWSYS_CHAR_IS_SIGNED
    "Checking whether char is signed" DIRECT)
ENDIF(KWSYS_USE_FundamentalType)

IF(KWSYS_USE_IOStream)
  # Determine whether iostreams support long long.
  KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_LONG_LONG
    "Checking whether C++ compiler has 'long long'" DIRECT)
  IF(KWSYS_CXX_HAS_LONG_LONG)
    SET(KWSYS_PLATFORM_CXX_TEST_DEFINES
      -DKWSYS_IOS_USE_ANSI=${KWSYS_IOS_USE_ANSI}
      -DKWSYS_IOS_HAVE_STD=${KWSYS_IOS_HAVE_STD})
    KWSYS_PLATFORM_CXX_TEST(KWSYS_IOS_HAS_ISTREAM_LONG_LONG
      "Checking if istream supports long long" DIRECT)
    KWSYS_PLATFORM_CXX_TEST(KWSYS_IOS_HAS_OSTREAM_LONG_LONG
      "Checking if ostream supports long long" DIRECT)
    SET(KWSYS_PLATFORM_CXX_TEST_DEFINES)
  ELSE()
    SET(KWSYS_IOS_HAS_ISTREAM_LONG_LONG 0)
    SET(KWSYS_IOS_HAS_OSTREAM_LONG_LONG 0)
  ENDIF()
ENDIF(KWSYS_USE_IOStream)

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
# Configure some implementation details.

KWSYS_PLATFORM_C_TEST(KWSYS_C_HAS_PTRDIFF_T
  "Checking whether C compiler has ptrdiff_t in stddef.h" DIRECT)
KWSYS_PLATFORM_C_TEST(KWSYS_C_HAS_SSIZE_T
  "Checking whether C compiler has ssize_t in unistd.h" DIRECT)
SET_SOURCE_FILES_PROPERTIES(ProcessUNIX.c System.c PROPERTIES
  COMPILE_FLAGS "-DKWSYS_C_HAS_PTRDIFF_T=${KWSYS_C_HAS_PTRDIFF_T} -DKWSYS_C_HAS_SSIZE_T=${KWSYS_C_HAS_SSIZE_T}"
  )

IF(KWSYS_DO_NOT_CLEAN_PUTENV)
  # Disable cleanup of putenv memory for issues with GCOV.
  SET_SOURCE_FILES_PROPERTIES(SystemTools.cxx PROPERTIES
    COMPILE_FLAGS -DKWSYS_DO_NOT_CLEAN_PUTENV=1)
ENDIF(KWSYS_DO_NOT_CLEAN_PUTENV)

#-----------------------------------------------------------------------------
# Choose a directory for the generated headers.
IF(NOT KWSYS_HEADER_ROOT)
  SET(KWSYS_HEADER_ROOT "${PROJECT_BINARY_DIR}")
ENDIF(NOT KWSYS_HEADER_ROOT)
SET(KWSYS_HEADER_DIR "${KWSYS_HEADER_ROOT}/${KWSYS_NAMESPACE}")
INCLUDE_DIRECTORIES(${KWSYS_HEADER_ROOT})

#-----------------------------------------------------------------------------
IF(KWSYS_INSTALL_DOC_DIR)
  # Assign the license to the runtime component since it must be
  # distributed with binary forms of this software.
  IF(KWSYS_INSTALL_COMPONENT_NAME_RUNTIME)
    SET(KWSYS_INSTALL_LICENSE_OPTIONS ${KWSYS_INSTALL_LICENSE_OPTIONS}
      COMPONENT ${KWSYS_INSTALL_COMPONENT_NAME_RUNTIME}
      )
  ENDIF(KWSYS_INSTALL_COMPONENT_NAME_RUNTIME)

  # Install the license under the documentation directory.
  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt
    DESTINATION ${KWSYS_INSTALL_DOC_DIR}/${KWSYS_NAMESPACE}
    ${KWSYS_INSTALL_LICENSE_OPTIONS})
ENDIF(KWSYS_INSTALL_DOC_DIR)

#-----------------------------------------------------------------------------
# Create STL header wrappers to block warnings in the STL headers and
# give standard names by which they may be included.
SET(KWSYS_STL_HEADER_EXTRA_string 1)
FOREACH(header
  algorithm
  deque
  exception
  functional
  iterator
  list
  map
  memory
  new
  numeric
  queue
  set
  stack
  stdexcept
  string
  utility
  vector
  )
  # Configure the header wrapper.
  SET(KWSYS_STL_HEADER "${header}")
  IF(KWSYS_STL_HEADER_EXTRA_${header})
    SET(KWSYS_STL_HEADER_EXTRA
      "#define ${KWSYS_NAMESPACE}_stl_${header}_including_hxx\n# include <${KWSYS_NAMESPACE}/stl/${header}.hxx>\n#undef ${KWSYS_NAMESPACE}_stl_${header}_including_hxx\n")
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/kwsys_stl_${header}.hxx.in
                   ${KWSYS_HEADER_DIR}/stl/${header}.hxx
                   @ONLY IMMEDIATE)
    IF(KWSYS_INSTALL_INCLUDE_DIR)
      INSTALL(FILES ${KWSYS_HEADER_DIR}/stl/${header}.hxx
        DESTINATION ${KWSYS_INSTALL_INCLUDE_DIR}/${KWSYS_NAMESPACE}/stl
        ${KWSYS_INSTALL_INCLUDE_OPTIONS})
    ENDIF(KWSYS_INSTALL_INCLUDE_DIR)
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
  IF(KWSYS_INSTALL_INCLUDE_DIR)
    INSTALL(FILES ${KWSYS_HEADER_DIR}/stl/${header}
      DESTINATION ${KWSYS_INSTALL_INCLUDE_DIR}/${KWSYS_NAMESPACE}/stl
      ${KWSYS_INSTALL_INCLUDE_OPTIONS})
  ENDIF(KWSYS_INSTALL_INCLUDE_DIR)
  IF(KWSYS_HEADER_INSTALL_DIR)
    INSTALL_FILES(${KWSYS_HEADER_INSTALL_DIR}/${KWSYS_NAMESPACE}/stl
      FILES ${KWSYS_HEADER_DIR}/stl/${header})
  ENDIF(KWSYS_HEADER_INSTALL_DIR)
ENDFOREACH(header)

# Provide cstddef header.
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/kwsys_cstddef.hxx.in
               ${KWSYS_HEADER_DIR}/cstddef
               @ONLY IMMEDIATE)
IF(KWSYS_INSTALL_INCLUDE_DIR)
  INSTALL(FILES ${KWSYS_HEADER_DIR}/cstddef
    DESTINATION ${KWSYS_INSTALL_INCLUDE_DIR}/${KWSYS_NAMESPACE}
    ${KWSYS_INSTALL_INCLUDE_OPTIONS})
  IF(KWSYS_HEADER_INSTALL_DIR)
    INSTALL_FILES(${KWSYS_HEADER_INSTALL_DIR}/${KWSYS_NAMESPACE}
      FILES ${KWSYS_HEADER_DIR}/cstddef)
  ENDIF(KWSYS_HEADER_INSTALL_DIR)
ENDIF(KWSYS_INSTALL_INCLUDE_DIR)

#-----------------------------------------------------------------------------
# Create streams header wrappers to give standard names by which they
# may be included.
FOREACH(header iostream fstream sstream iosfwd)
  # Configure the header wrapper.
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/kwsys_ios_${header}.h.in
                 ${KWSYS_HEADER_DIR}/ios/${header}
                 @ONLY IMMEDIATE)

  # Create an install target for the header wrapper.
  IF(KWSYS_INSTALL_INCLUDE_DIR)
    INSTALL(FILES ${KWSYS_HEADER_DIR}/ios/${header}
      DESTINATION ${KWSYS_INSTALL_INCLUDE_DIR}/${KWSYS_NAMESPACE}/ios
      ${KWSYS_INSTALL_INCLUDE_OPTIONS})
  ENDIF(KWSYS_INSTALL_INCLUDE_DIR)
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

# Add selected C++ classes.
SET(cppclasses
  Directory DynamicLoader Glob RegularExpression SystemTools
  CommandLineArguments Registry IOStream SystemInformation
  )
FOREACH(cpp ${cppclasses})
  IF(KWSYS_USE_${cpp})
    # Use the corresponding class.
    SET(KWSYS_CLASSES ${KWSYS_CLASSES} ${cpp})

    # Load component-specific CMake code.
    IF(EXISTS ${PROJECT_SOURCE_DIR}/kwsys${cpp}.cmake)
      INCLUDE(${PROJECT_SOURCE_DIR}/kwsys${cpp}.cmake)
    ENDIF(EXISTS ${PROJECT_SOURCE_DIR}/kwsys${cpp}.cmake)
  ENDIF(KWSYS_USE_${cpp})
ENDFOREACH(cpp)

# Add selected C components.
FOREACH(c
    Process Base64 FundamentalType MD5 Terminal System DateStamp String CPU
    )
  IF(KWSYS_USE_${c})
    # Use the corresponding header file.
    SET(KWSYS_H_FILES ${KWSYS_H_FILES} ${c})

    # Load component-specific CMake code.
    IF(EXISTS ${PROJECT_SOURCE_DIR}/kwsys${c}.cmake)
      INCLUDE(${PROJECT_SOURCE_DIR}/kwsys${c}.cmake)
    ENDIF(EXISTS ${PROJECT_SOURCE_DIR}/kwsys${c}.cmake)
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

# Add selected C sources.
FOREACH(c Base64 MD5 Terminal System String)
  IF(KWSYS_USE_${c})
    SET(KWSYS_C_SRCS ${KWSYS_C_SRCS} ${c}.c)
  ENDIF(KWSYS_USE_${c})
ENDFOREACH(c)

# Configure headers of C++ classes and construct the list of sources.
FOREACH(c ${KWSYS_CLASSES})
  # Add this source to the list of source files for the library.
  SET(KWSYS_CXX_SRCS ${KWSYS_CXX_SRCS} ${c}.cxx)

  # Configure the header for this class.
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/${c}.hxx.in ${KWSYS_HEADER_DIR}/${c}.hxx
                 @ONLY IMMEDIATE)
  SET(KWSYS_CXX_SRCS ${KWSYS_CXX_SRCS} ${KWSYS_HEADER_DIR}/${c}.hxx)

  # Create an install target for the header.
  IF(KWSYS_INSTALL_INCLUDE_DIR)
    INSTALL(FILES ${KWSYS_HEADER_DIR}/${c}.hxx
      DESTINATION ${KWSYS_INSTALL_INCLUDE_DIR}/${KWSYS_NAMESPACE}
      ${KWSYS_INSTALL_INCLUDE_OPTIONS})
  ENDIF(KWSYS_INSTALL_INCLUDE_DIR)
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
  SET(KWSYS_C_SRCS ${KWSYS_C_SRCS} ${KWSYS_HEADER_DIR}/${h}.h)

  # Create an install target for the header.
  IF(KWSYS_INSTALL_INCLUDE_DIR)
    INSTALL(FILES ${KWSYS_HEADER_DIR}/${h}.h
      DESTINATION ${KWSYS_INSTALL_INCLUDE_DIR}/${KWSYS_NAMESPACE}
      ${KWSYS_INSTALL_INCLUDE_OPTIONS})
  ENDIF(KWSYS_INSTALL_INCLUDE_DIR)
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
  SET(KWSYS_CXX_SRCS ${KWSYS_CXX_SRCS} ${KWSYS_HEADER_DIR}/${h}.hxx)

  # Create an install target for the header.
  IF(KWSYS_INSTALL_INCLUDE_DIR)
    INSTALL(FILES ${KWSYS_HEADER_DIR}/${h}.hxx
      DESTINATION ${KWSYS_INSTALL_INCLUDE_DIR}/${KWSYS_NAMESPACE}
      ${KWSYS_INSTALL_INCLUDE_OPTIONS})
  ENDIF(KWSYS_INSTALL_INCLUDE_DIR)
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
  KWSYS_SET_PROPERTY(TARGET ${KWSYS_NAMESPACE} PROPERTY LABELS ${KWSYS_LABELS_LIB})
  IF(KWSYS_USE_DynamicLoader)
    IF(UNIX)
      TARGET_LINK_LIBRARIES(${KWSYS_NAMESPACE} ${CMAKE_DL_LIBS})
    ENDIF(UNIX)
  ENDIF(KWSYS_USE_DynamicLoader)
  
  IF(KWSYS_USE_SystemInformation AND WIN32)
    TARGET_LINK_LIBRARIES(${KWSYS_NAMESPACE} ws2_32)
  ENDIF(KWSYS_USE_SystemInformation AND WIN32)

  # Apply user-defined target properties to the library.
  IF(KWSYS_PROPERTIES_CXX)
    SET_TARGET_PROPERTIES(${KWSYS_NAMESPACE} PROPERTIES
      ${KWSYS_PROPERTIES_CXX}
      )
  ENDIF(KWSYS_PROPERTIES_CXX)

  # Create an install target for the library.
  IF(KWSYS_INSTALL_LIBRARY_RULE)
    INSTALL(TARGETS ${KWSYS_NAMESPACE} ${KWSYS_INSTALL_LIBRARY_RULE})
  ENDIF(KWSYS_INSTALL_LIBRARY_RULE)
  IF(KWSYS_LIBRARY_INSTALL_DIR)
    INSTALL_TARGETS(${KWSYS_LIBRARY_INSTALL_DIR} ${KWSYS_NAMESPACE})
  ENDIF(KWSYS_LIBRARY_INSTALL_DIR)
ENDIF(KWSYS_C_SRCS OR KWSYS_CXX_SRCS)

# Add a C-only library if requested.
IF(KWSYS_ENABLE_C AND KWSYS_C_SRCS)
  ADD_LIBRARY(${KWSYS_NAMESPACE}_c ${KWSYS_LIBRARY_TYPE} ${KWSYS_C_SRCS})
  KWSYS_SET_PROPERTY(TARGET ${KWSYS_NAMESPACE}_c PROPERTY LABELS ${KWSYS_LABELS_LIB})

  # Apply user-defined target properties to the library.
  IF(KWSYS_PROPERTIES_C)
    SET_TARGET_PROPERTIES(${KWSYS_NAMESPACE} PROPERTIES
      ${KWSYS_PROPERTIES_C}
      )
  ENDIF(KWSYS_PROPERTIES_C)

  # Create an install target for the library.
  IF(KWSYS_INSTALL_LIBRARY_RULE)
    INSTALL(TARGETS ${KWSYS_NAMESPACE}_c ${KWSYS_INSTALL_LIBRARY_RULE})
  ENDIF(KWSYS_INSTALL_LIBRARY_RULE)
  IF(KWSYS_LIBRARY_INSTALL_DIR)
    INSTALL_TARGETS(${KWSYS_LIBRARY_INSTALL_DIR} ${KWSYS_NAMESPACE}_c)
  ENDIF(KWSYS_LIBRARY_INSTALL_DIR)
ENDIF(KWSYS_ENABLE_C AND KWSYS_C_SRCS)

# For building kwsys itself, we use a macro defined on the command
# line to configure the namespace in the C and C++ source files.
ADD_DEFINITIONS("-DKWSYS_NAMESPACE=${KWSYS_NAMESPACE}")

IF(KWSYS_USE_String)
  # Activate code in "String.c".  See the comment in the source.
  SET_SOURCE_FILES_PROPERTIES(String.c PROPERTIES
    COMPILE_FLAGS "-DKWSYS_STRING_C")
ENDIF(KWSYS_USE_String)

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
    KWSYS_SET_PROPERTY(TARGET ${KWSYS_NAMESPACE}ProcessFwd9x PROPERTY LABELS ${KWSYS_LABELS_EXE})
    KWSYS_SET_PROPERTY(TARGET ${KWSYS_NAMESPACE}EncodeExecutable PROPERTY LABELS ${KWSYS_LABELS_EXE})

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
    # Compute the location of executables.
    SET(EXEC_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    IF(EXECUTABLE_OUTPUT_PATH)
      SET(EXEC_DIR "${EXECUTABLE_OUTPUT_PATH}")
    ENDIF(EXECUTABLE_OUTPUT_PATH)

    # C tests
    SET(KWSYS_C_TESTS
      testEncode
      testTerminal
      )
    IF(COMMAND SET_TESTS_PROPERTIES AND COMMAND GET_TEST_PROPERTY AND KWSYS_STANDALONE)
      SET(KWSYS_C_TESTS ${KWSYS_C_TESTS} testFail)
    ENDIF(COMMAND SET_TESTS_PROPERTIES AND COMMAND GET_TEST_PROPERTY AND KWSYS_STANDALONE)
    CREATE_TEST_SOURCELIST(
      KWSYS_C_TEST_SRCS ${KWSYS_NAMESPACE}TestsC.c
      ${KWSYS_C_TESTS}
      )
    ADD_EXECUTABLE(${KWSYS_NAMESPACE}TestsC ${KWSYS_C_TEST_SRCS})
    KWSYS_SET_PROPERTY(TARGET ${KWSYS_NAMESPACE}TestsC PROPERTY LABELS ${KWSYS_LABELS_EXE})
    TARGET_LINK_LIBRARIES(${KWSYS_NAMESPACE}TestsC ${KWSYS_NAMESPACE}_c)
    FOREACH(test ${KWSYS_C_TESTS})
      ADD_TEST(kwsys.${test} ${EXEC_DIR}/${KWSYS_NAMESPACE}TestsC ${test} ${KWSYS_TEST_ARGS_${test}})
      KWSYS_SET_PROPERTY(TEST kwsys.${test} PROPERTY LABELS ${KWSYS_LABELS_TEST})
    ENDFOREACH(test)

    # C++ tests
    IF(NOT WATCOM)
      SET(KWSYS_CXX_TESTS
        testAutoPtr
        testHashSTL
        )
    ENDIF(NOT WATCOM)
    SET(KWSYS_CXX_TESTS ${KWSYS_CXX_TESTS}
      testRegistry
      testIOS
      testSystemTools
      testCommandLineArguments
      testCommandLineArguments1
      )
    IF(KWSYS_USE_SystemInformation)
      SET(KWSYS_CXX_TESTS ${KWSYS_CXX_TESTS} testSystemInformation)
    ENDIF(KWSYS_USE_SystemInformation)
    IF(KWSYS_USE_DynamicLoader)
      SET(KWSYS_CXX_TESTS ${KWSYS_CXX_TESTS} testDynamicLoader)
      # If kwsys contains the DynamicLoader, need extra library
      ADD_LIBRARY(${KWSYS_NAMESPACE}TestDynload MODULE testDynload.c)
      KWSYS_SET_PROPERTY(TARGET ${KWSYS_NAMESPACE}TestDynload PROPERTY LABELS ${KWSYS_LABELS_LIB})
      ADD_DEPENDENCIES(${KWSYS_NAMESPACE}TestDynload ${KWSYS_NAMESPACE})
    ENDIF(KWSYS_USE_DynamicLoader)
    CREATE_TEST_SOURCELIST(
      KWSYS_CXX_TEST_SRCS ${KWSYS_NAMESPACE}TestsCxx.cxx
      ${KWSYS_CXX_TESTS}
      )
    ADD_EXECUTABLE(${KWSYS_NAMESPACE}TestsCxx ${KWSYS_CXX_TEST_SRCS})
    KWSYS_SET_PROPERTY(TARGET ${KWSYS_NAMESPACE}TestsCxx PROPERTY LABELS ${KWSYS_LABELS_EXE})
    TARGET_LINK_LIBRARIES(${KWSYS_NAMESPACE}TestsCxx ${KWSYS_NAMESPACE})
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

    SET(KWSYS_TEST_ARGS_testCommandLineArguments
      --another-bool-variable
      --long3=opt
      --set-bool-arg1
      -SSS ken brad bill andy
      --some-bool-variable=true
      --some-double-variable12.5
      --some-int-variable 14
      "--some-string-variable=test string with space"
      --some-multi-argument 5 1 8 3 7 1 3 9 7 1
      -N 12.5 -SS=andy -N 1.31 -N 22
      -SS=bill -BBtrue -SS=brad
      -BBtrue
      -BBfalse
      -SS=ken
      -A
      -C=test
      --long2 hello
      )
    SET(KWSYS_TEST_ARGS_testCommandLineArguments1
      --ignored
      -n 24
      --second-ignored
      "-m=test value"
      third-ignored
      -p
      some junk at the end
      )
    FOREACH(test ${KWSYS_CXX_TESTS})
      ADD_TEST(kwsys.${test} ${EXEC_DIR}/${KWSYS_NAMESPACE}TestsCxx ${test} ${KWSYS_TEST_ARGS_${test}})
      KWSYS_SET_PROPERTY(TEST kwsys.${test} PROPERTY LABELS ${KWSYS_LABELS_TEST})
    ENDFOREACH(test)

    # Process tests.
    ADD_EXECUTABLE(${KWSYS_NAMESPACE}TestProcess testProcess.c)
    KWSYS_SET_PROPERTY(TARGET ${KWSYS_NAMESPACE}TestProcess PROPERTY LABELS ${KWSYS_LABELS_EXE})
    TARGET_LINK_LIBRARIES(${KWSYS_NAMESPACE}TestProcess ${KWSYS_NAMESPACE}_c)
    IF(NOT CYGWIN)
      SET(KWSYS_TEST_PROCESS_7 7)
    ENDIF(NOT CYGWIN)
    FOREACH(n 1 2 3 4 5 6 ${KWSYS_TEST_PROCESS_7})
      ADD_TEST(kwsys.testProcess-${n} ${EXEC_DIR}/${KWSYS_NAMESPACE}TestProcess ${n})
      KWSYS_SET_PROPERTY(TEST kwsys.testProcess-${n} PROPERTY LABELS ${KWSYS_LABELS_TEST})
      SET_TESTS_PROPERTIES(kwsys.testProcess-${n} PROPERTIES TIMEOUT 120)
    ENDFOREACH(n)

    # Some Apple compilers produce bad optimizations in this source.
    IF(APPLE AND "${CMAKE_C_COMPILER_ID}" MATCHES "^(GNU|LLVM)$")
      SET_SOURCE_FILES_PROPERTIES(testProcess.c PROPERTIES COMPILE_FLAGS -O0)
    ENDIF()

    # Test SharedForward
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/testSharedForward.c.in
                   ${PROJECT_BINARY_DIR}/testSharedForward.c @ONLY IMMEDIATE)
    ADD_EXECUTABLE(${KWSYS_NAMESPACE}TestSharedForward
                   ${PROJECT_BINARY_DIR}/testSharedForward.c)
    KWSYS_SET_PROPERTY(TARGET ${KWSYS_NAMESPACE}TestSharedForward PROPERTY LABELS ${KWSYS_LABELS_EXE})
    ADD_DEPENDENCIES(${KWSYS_NAMESPACE}TestSharedForward ${KWSYS_NAMESPACE}_c)
    ADD_TEST(kwsys.testSharedForward ${EXEC_DIR}/${KWSYS_NAMESPACE}TestSharedForward 1)
    KWSYS_SET_PROPERTY(TEST kwsys.testSharedForward PROPERTY LABELS ${KWSYS_LABELS_TEST})

    # Configure some test properties.
    IF(COMMAND SET_TESTS_PROPERTIES AND COMMAND GET_TEST_PROPERTY AND KWSYS_STANDALONE)
      # We expect test to fail
      SET_TESTS_PROPERTIES(kwsys.testFail PROPERTIES WILL_FAIL ON)
      GET_TEST_PROPERTY(kwsys.testFail WILL_FAIL wfv)
      SET_TESTS_PROPERTIES(kwsys.testRegistry PROPERTIES FAIL_REGULAR_EXPRESSION "ERROR_NOT_VALGRIND;FAIL;Test failed")
      SET_TESTS_PROPERTIES(kwsys.testRegistry PROPERTIES PASS_REGULAR_EXPRESSION "Test passed")
      SET_TESTS_PROPERTIES(kwsys.testFail PROPERTIES MEASUREMENT "Some Key=Some Value")
      MESSAGE(STATUS "GET_TEST_PROPERTY returned: ${wfv}")
    ENDIF(COMMAND SET_TESTS_PROPERTIES AND COMMAND GET_TEST_PROPERTY AND KWSYS_STANDALONE)

    # Suppress known consistent failures on buggy systems.
    IF(KWSYS_TEST_BOGUS_FAILURES)
      SET_TESTS_PROPERTIES(${KWSYS_TEST_BOGUS_FAILURES} PROPERTIES WILL_FAIL ON)
    ENDIF()

  ENDIF(BUILD_TESTING)
ENDIF(KWSYS_STANDALONE OR CMake_SOURCE_DIR)
