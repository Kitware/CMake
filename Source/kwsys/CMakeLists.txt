#=============================================================================
# KWSys - Kitware System Library
# Copyright 2000-2011 Kitware, Inc., Insight Software Consortium
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
#  set(KWSYS_NAMESPACE foosys)
#  set(KWSYS_USE_Directory 1)    # Enable Directory class.
#  subdirs(kwsys)
#
# Optional settings are as follows:
#
#  KWSYS_HEADER_ROOT = The directory into which to generate the kwsys headers.
#                      A directory called "${KWSYS_NAMESPACE}" will be
#                      created under this root directory to hold the files.
#
#    Example:
#
#      set(KWSYS_HEADER_ROOT ${PROJECT_BINARY_DIR})
#      include_directories(${PROJECT_BINARY_DIR})
#
#  KWSYS_IOS_FORCE_OLD = Force use of old non-ANSI C++ streams even if
#                        new streams are available.  This may be used
#                        by projects that cannot configure their
#                        streams library.
#    Example:
#
#      set(KWSYS_IOS_FORCE_OLD 1)
#
#
# Optional settings to setup install rules are as follows:
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
#      set(KWSYS_INSTALL_BIN_DIR bin)
#      set(KWSYS_INSTALL_LIB_DIR lib)
#      set(KWSYS_INSTALL_INCLUDE_DIR include)
#      set(KWSYS_INSTALL_COMPONENT_NAME_RUNTIME Runtime)
#      set(KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT Development)

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

cmake_minimum_required(VERSION 2.6.3 FATAL_ERROR)

#-----------------------------------------------------------------------------
# If a namespace is not specified, use "kwsys" and enable testing.
# This should be the case only when kwsys is not included inside
# another project and is being tested.
if(NOT KWSYS_NAMESPACE)
  set(KWSYS_NAMESPACE "kwsys")
  set(KWSYS_STANDALONE 1)
endif(NOT KWSYS_NAMESPACE)

#-----------------------------------------------------------------------------
# The project name is that of the specified namespace.
project(${KWSYS_NAMESPACE})

# Tell CMake how to follow dependencies of sources in this directory.
set_property(DIRECTORY
  PROPERTY IMPLICIT_DEPENDS_INCLUDE_TRANSFORM
  "KWSYS_HEADER(%)=<${KWSYS_NAMESPACE}/%>"
  )

# Select library components.
if(KWSYS_STANDALONE OR CMake_SOURCE_DIR)
  set(KWSYS_ENABLE_C 1)
  # Enable all components.
  set(KWSYS_USE_Base64 1)
  set(KWSYS_USE_Directory 1)
  set(KWSYS_USE_DynamicLoader 1)
  set(KWSYS_USE_Glob 1)
  set(KWSYS_USE_MD5 1)
  set(KWSYS_USE_Process 1)
  set(KWSYS_USE_RegularExpression 1)
  set(KWSYS_USE_Registry 1)
  set(KWSYS_USE_System 1)
  set(KWSYS_USE_SystemTools 1)
  set(KWSYS_USE_CommandLineArguments 1)
  set(KWSYS_USE_FundamentalType 1)
  set(KWSYS_USE_Terminal 1)
  set(KWSYS_USE_IOStream 1)
  set(KWSYS_USE_DateStamp 1)
  set(KWSYS_USE_String 1)
  set(KWSYS_USE_SystemInformation 1)
  set(KWSYS_USE_CPU 1)
endif(KWSYS_STANDALONE OR CMake_SOURCE_DIR)

# Enforce component dependencies.
if(KWSYS_USE_SystemTools)
  set(KWSYS_USE_Directory 1)
endif(KWSYS_USE_SystemTools)
if(KWSYS_USE_Glob)
  set(KWSYS_USE_Directory 1)
  set(KWSYS_USE_SystemTools 1)
  set(KWSYS_USE_RegularExpression 1)
endif(KWSYS_USE_Glob)
if(KWSYS_USE_Process)
  set(KWSYS_USE_System 1)
endif(KWSYS_USE_Process)
if(KWSYS_USE_SystemInformation)
  set(KWSYS_USE_FundamentalType 1)
  set(KWSYS_USE_Process 1)
endif(KWSYS_USE_SystemInformation)

# Setup the large file support default.
if(KWSYS_LFS_DISABLE)
  set(KWSYS_LFS_REQUESTED 0)
else(KWSYS_LFS_DISABLE)
  set(KWSYS_LFS_REQUESTED 1)
endif(KWSYS_LFS_DISABLE)

# Enable testing if building standalone.
if(KWSYS_STANDALONE)
  include(Dart)
  mark_as_advanced(BUILD_TESTING DART_ROOT TCL_TCLSH)
  if(BUILD_TESTING)
    enable_testing()
  endif(BUILD_TESTING)
endif(KWSYS_STANDALONE)

# Include helper macros.
include(${CMAKE_CURRENT_SOURCE_DIR}/kwsysPlatformTests.cmake)
include(CheckTypeSize)

# Do full dependency headers.
include_regular_expression("^.*$")

# Use new KWSYS_INSTALL_*_DIR variable names to control installation.
# Take defaults from the old names.  Note that there was no old name
# for the bin dir, so we take the old lib dir name so DLLs will be
# installed in a compatible way for old code.
if(NOT KWSYS_INSTALL_INCLUDE_DIR)
  string(REGEX REPLACE "^/" "" KWSYS_INSTALL_INCLUDE_DIR
    "${KWSYS_HEADER_INSTALL_DIR}")
endif(NOT KWSYS_INSTALL_INCLUDE_DIR)
if(NOT KWSYS_INSTALL_LIB_DIR)
  string(REGEX REPLACE "^/" "" KWSYS_INSTALL_LIB_DIR
    "${KWSYS_LIBRARY_INSTALL_DIR}")
endif(NOT KWSYS_INSTALL_LIB_DIR)
if(NOT KWSYS_INSTALL_BIN_DIR)
  string(REGEX REPLACE "^/" "" KWSYS_INSTALL_BIN_DIR
    "${KWSYS_LIBRARY_INSTALL_DIR}")
endif(NOT KWSYS_INSTALL_BIN_DIR)

# Setup header install rules.
set(KWSYS_INSTALL_INCLUDE_OPTIONS)
if(KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT)
  set(KWSYS_INSTALL_INCLUDE_OPTIONS ${KWSYS_INSTALL_INCLUDE_OPTIONS}
    COMPONENT ${KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT}
    )
endif(KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT)

# Setup library install rules.
set(KWSYS_INSTALL_LIBRARY_RULE)
if(KWSYS_INSTALL_LIB_DIR)
  if(KWSYS_INSTALL_EXPORT_NAME)
    list(APPEND KWSYS_INSTALL_LIBRARY_RULE EXPORT ${KWSYS_INSTALL_EXPORT_NAME})
  endif()
  # Install the shared library to the lib directory.
  set(KWSYS_INSTALL_LIBRARY_RULE ${KWSYS_INSTALL_LIBRARY_RULE}
    LIBRARY DESTINATION ${KWSYS_INSTALL_LIB_DIR}
    )
  # Assign the shared library to the runtime component.
  if(KWSYS_INSTALL_COMPONENT_NAME_RUNTIME)
    set(KWSYS_INSTALL_LIBRARY_RULE ${KWSYS_INSTALL_LIBRARY_RULE}
      COMPONENT ${KWSYS_INSTALL_COMPONENT_NAME_RUNTIME}
      )
  endif(KWSYS_INSTALL_COMPONENT_NAME_RUNTIME)

  # Install the archive to the lib directory.
  set(KWSYS_INSTALL_LIBRARY_RULE ${KWSYS_INSTALL_LIBRARY_RULE}
    ARCHIVE DESTINATION ${KWSYS_INSTALL_LIB_DIR}
    )
  # Assign the archive to the development component.
  if(KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT)
    set(KWSYS_INSTALL_LIBRARY_RULE ${KWSYS_INSTALL_LIBRARY_RULE}
      COMPONENT ${KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT}
      )
  endif(KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT)
endif(KWSYS_INSTALL_LIB_DIR)
if(KWSYS_INSTALL_BIN_DIR)
  # Install the runtime library to the bin directory.
  set(KWSYS_INSTALL_LIBRARY_RULE ${KWSYS_INSTALL_LIBRARY_RULE}
    RUNTIME DESTINATION ${KWSYS_INSTALL_BIN_DIR}
    )
  # Assign the runtime library to the runtime component.
  if(KWSYS_INSTALL_COMPONENT_NAME_RUNTIME)
    set(KWSYS_INSTALL_LIBRARY_RULE ${KWSYS_INSTALL_LIBRARY_RULE}
      COMPONENT ${KWSYS_INSTALL_COMPONENT_NAME_RUNTIME}
      )
  endif(KWSYS_INSTALL_COMPONENT_NAME_RUNTIME)
endif(KWSYS_INSTALL_BIN_DIR)

# Do not support old KWSYS_*a_INSTALL_DIR variable names.
set(KWSYS_HEADER_INSTALL_DIR)
set(KWSYS_LIBRARY_INSTALL_DIR)

# Generated source files will need this header.
string(COMPARE EQUAL "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}"
  KWSYS_IN_SOURCE_BUILD)
if(NOT KWSYS_IN_SOURCE_BUILD)
  configure_file(${PROJECT_SOURCE_DIR}/kwsysPrivate.h
    ${PROJECT_BINARY_DIR}/kwsysPrivate.h COPY_ONLY IMMEDIATE)
endif(NOT KWSYS_IN_SOURCE_BUILD)

# Select plugin module file name convention.
if(NOT KWSYS_DynamicLoader_PREFIX)
  set(KWSYS_DynamicLoader_PREFIX ${CMAKE_SHARED_MODULE_PREFIX})
endif()
if(NOT KWSYS_DynamicLoader_SUFFIX)
  set(KWSYS_DynamicLoader_SUFFIX ${CMAKE_SHARED_MODULE_SUFFIX})
endif()

#-----------------------------------------------------------------------------
# We require ANSI support from the C compiler.  Add any needed flags.
if(CMAKE_ANSI_CFLAGS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")
endif(CMAKE_ANSI_CFLAGS)

#-----------------------------------------------------------------------------
# Adjust compiler flags for some platforms.
if(NOT CMAKE_COMPILER_IS_GNUCXX)
  if(CMAKE_SYSTEM MATCHES "OSF1-V.*")
    string(REGEX MATCH "-timplicit_local"
      KWSYS_CXX_FLAGS_HAVE_IMPLICIT_LOCAL "${CMAKE_CXX_FLAGS}")
    string(REGEX MATCH "-no_implicit_include"
      KWSYS_CXX_FLAGS_HAVE_NO_IMPLICIT_INCLUDE "${CMAKE_CXX_FLAGS}")
    if(NOT KWSYS_CXX_FLAGS_HAVE_IMPLICIT_LOCAL)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -timplicit_local")
    endif(NOT KWSYS_CXX_FLAGS_HAVE_IMPLICIT_LOCAL)
    if(NOT KWSYS_CXX_FLAGS_HAVE_NO_IMPLICIT_INCLUDE)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -no_implicit_include")
    endif(NOT KWSYS_CXX_FLAGS_HAVE_NO_IMPLICIT_INCLUDE)
  endif(CMAKE_SYSTEM MATCHES "OSF1-V.*")
  if(CMAKE_SYSTEM MATCHES "HP-UX")
    set(KWSYS_PLATFORM_CXX_TEST_EXTRA_FLAGS "+p")
  endif(CMAKE_SYSTEM MATCHES "HP-UX")
endif(NOT CMAKE_COMPILER_IS_GNUCXX)

#-----------------------------------------------------------------------------
# Configure Large File Support.
KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_CSTDIO
  "Checking whether header cstdio is available" DIRECT)
set(KWSYS_LFS_AVAILABLE 0)
if(KWSYS_LFS_REQUESTED)
  # Large File Support is requested.
  set(KWSYS_LFS_REQUESTED 1)

  # Check for large file support.
  set(KWSYS_PLATFORM_CXX_TEST_DEFINES
    -DKWSYS_CXX_HAS_CSTDIO=${KWSYS_CXX_HAS_CSTDIO})
  KWSYS_PLATFORM_CXX_TEST_RUN(KWSYS_LFS_WORKS
    "Checking for Large File Support" DIRECT)
  set(KWSYS_PLATFORM_CXX_TEST_DEFINES)

  if(KWSYS_LFS_WORKS)
    set(KWSYS_LFS_AVAILABLE 1)
  endif(KWSYS_LFS_WORKS)
else(KWSYS_LFS_REQUESTED)
  # Large File Support is not requested.
  set(KWSYS_LFS_REQUESTED 0)
endif(KWSYS_LFS_REQUESTED)

#-----------------------------------------------------------------------------
# Configure the standard library header wrappers based on compiler's
# capabilities and parent project's request.  Enforce 0/1 as only
# possible values for configuration into Configure.hxx.

KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAVE_STD
  "Checking whether STL classes are in std namespace" DIRECT)

if(KWSYS_IOS_FORCE_OLD)
  set(KWSYS_IOS_USE_ANSI 0)
else(KWSYS_IOS_FORCE_OLD)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_IOS_USE_ANSI
    "Checking whether ANSI stream headers are available" DIRECT)
endif(KWSYS_IOS_FORCE_OLD)

if(KWSYS_IOS_USE_ANSI)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_IOS_HAVE_STD
    "Checking whether ANSI streams are in std namespace" DIRECT)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_IOS_USE_SSTREAM
    "Checking whether ANSI string stream is available" DIRECT)
else(KWSYS_IOS_USE_ANSI)
  set(KWSYS_IOS_HAVE_STD 0)
  set(KWSYS_IOS_USE_SSTREAM 0)
endif(KWSYS_IOS_USE_ANSI)

if(KWSYS_IOS_USE_SSTREAM)
  set(KWSYS_IOS_USE_STRSTREAM_H 0)
  set(KWSYS_IOS_USE_STRSTREA_H 0)
else(KWSYS_IOS_USE_SSTREAM)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_IOS_USE_STRSTREAM_H
    "Checking whether strstream.h is available" DIRECT)
  if(KWSYS_IOS_USE_STRSTREAM_H)
    set(KWSYS_IOS_USE_STRSTREA_H 0)
  else(KWSYS_IOS_USE_STRSTREAM_H)
    KWSYS_PLATFORM_CXX_TEST(KWSYS_IOS_USE_STRSTREA_H
      "Checking whether strstrea.h is available" DIRECT)
  endif(KWSYS_IOS_USE_STRSTREAM_H)
endif(KWSYS_IOS_USE_SSTREAM)

KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_CSTDDEF
  "Checking whether header cstddef is available" DIRECT)

set(KWSYS_PLATFORM_CXX_TEST_DEFINES
  -DKWSYS_STL_HAVE_STD=${KWSYS_STL_HAVE_STD})
KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_STRING_HAVE_NEQ_CHAR
  "Checking whether stl string has operator!= for char*" DIRECT)
KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS_ITERATOR_TRAITS
  "Checking whether stl has iterator_traits" DIRECT)
if(KWSYS_STL_HAS_ITERATOR_TRAITS)
  set(KWSYS_STL_HAS_ITERATOR_CATEGORY 0)
  set(KWSYS_STL_HAS___ITERATOR_CATEGORY 0)
else(KWSYS_STL_HAS_ITERATOR_TRAITS)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS_ITERATOR_CATEGORY
    "Checking whether stl has old iterator_category" DIRECT)
  if(KWSYS_STL_HAS_ITERATOR_CATEGORY)
    set(KWSYS_STL_HAS___ITERATOR_CATEGORY 0)
  else(KWSYS_STL_HAS_ITERATOR_CATEGORY)
    KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS___ITERATOR_CATEGORY
      "Checking whether stl has internal __iterator_category" DIRECT)
  endif(KWSYS_STL_HAS_ITERATOR_CATEGORY)
endif(KWSYS_STL_HAS_ITERATOR_TRAITS)
KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS_ALLOCATOR_TEMPLATE
  "Checking whether stl has standard template allocator" DIRECT)
if(KWSYS_STL_HAS_ALLOCATOR_TEMPLATE)
  set(KWSYS_STL_HAS_ALLOCATOR_NONTEMPLATE 0)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS_ALLOCATOR_REBIND
    "Checking for rebind member of stl allocator" DIRECT)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS_ALLOCATOR_MAX_SIZE_ARGUMENT
    "Checking for non-standard argument to stl allocator<>::max_size" DIRECT)
else(KWSYS_STL_HAS_ALLOCATOR_TEMPLATE)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS_ALLOCATOR_NONTEMPLATE
    "Checking whether stl has old non-template allocator" DIRECT)
  set(KWSYS_STL_HAS_ALLOCATOR_REBIND 0)
  set(KWSYS_STL_HAS_ALLOCATOR_MAX_SIZE_ARGUMENT 0)
endif(KWSYS_STL_HAS_ALLOCATOR_TEMPLATE)
KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS_ALLOCATOR_OBJECTS
  "Checking whether stl containers support allocator objects." DIRECT)
if(KWSYS_IOS_USE_ANSI AND NOT WATCOM)
  # ANSI streams always have string operators.
  set(KWSYS_STL_STRING_HAVE_OSTREAM 1)
  set(KWSYS_STL_STRING_HAVE_ISTREAM 1)
else(KWSYS_IOS_USE_ANSI AND NOT WATCOM)
  # There may not be string operators for old streams.
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_STRING_HAVE_OSTREAM
    "Checking whether stl string has ostream operator<<" DIRECT)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_STRING_HAVE_ISTREAM
    "Checking whether stl string has istream operator>>" DIRECT)
endif(KWSYS_IOS_USE_ANSI AND NOT WATCOM)
set(KWSYS_PLATFORM_CXX_TEST_DEFINES
  -DKWSYS_IOS_USE_ANSI=${KWSYS_IOS_USE_ANSI}
  -DKWSYS_IOS_HAVE_STD=${KWSYS_IOS_HAVE_STD})
KWSYS_PLATFORM_CXX_TEST(KWSYS_IOS_HAVE_BINARY
  "Checking whether ios has binary openmode" DIRECT)
set(KWSYS_PLATFORM_CXX_TEST_DEFINES)

KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_NULL_TEMPLATE_ARGS
  "Checking whether \"<>\" is needed for template friends" INVERT)
KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_MEMBER_TEMPLATES
  "Checking for member template support" DIRECT)
KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_FULL_SPECIALIZATION
  "Checking for standard template specialization syntax" DIRECT)
KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_ARGUMENT_DEPENDENT_LOOKUP
  "Checking whether argument dependent lookup is supported" DIRECT)

if(UNIX)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STAT_HAS_ST_MTIM
    "Checking whether struct stat has st_mtim member" DIRECT)
endif(UNIX)

if(KWSYS_USE_FundamentalType)
  # Look for type size helper macros.
  KWSYS_PLATFORM_INFO_TEST(C KWSYS_C_TYPE_MACROS
    "Checking for C type size macros")
  set(macro_regex ".*INFO:macro\\[([^]]*)\\].*")
  foreach(info ${KWSYS_C_TYPE_MACROS})
    if("${info}" MATCHES "${macro_regex}")
      string(REGEX REPLACE "${macro_regex}" "\\1" macro "${info}")
      set(KWSYS_C_HAS_MACRO_${macro} 1)
    endif()
  endforeach()

  # Determine type sizes at preprocessing time if possible, and
  # otherwise fall back to a try-compile.
  set(KWSYS_C_TYPE_NAME_CHAR      "char")
  set(KWSYS_C_TYPE_NAME_SHORT     "short")
  set(KWSYS_C_TYPE_NAME_INT       "int")
  set(KWSYS_C_TYPE_NAME_LONG      "long")
  set(KWSYS_C_TYPE_NAME_LONG_LONG "long long")
  set(KWSYS_C_TYPE_NAME___INT64   "__int64")
  foreach(type CHAR SHORT INT LONG LONG_LONG __INT64)
    if(KWSYS_C_HAS_MACRO___SIZEOF_${type}__)
      # Use __SIZEOF_${type}__ macro.
      set(KWSYS_SIZEOF_${type} TRUE)
      set(KWSYS_C_CODE_SIZEOF_${type} "#define ${KWSYS_NAMESPACE}_SIZEOF_${type} __SIZEOF_${type}__")
    elseif(KWSYS_C_HAS_MACRO___${type}_MAX__)
      # Use __${type}_MAX__ macro.
      set(KWSYS_SIZEOF_${type} TRUE)
      set(KWSYS_C_CODE_SIZEOF_${type} "#if __${type}_MAX__ == 0x7f
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
    else()
      # Configure a hard-coded type size.
      CHECK_TYPE_SIZE("${KWSYS_C_TYPE_NAME_${type}}" KWSYS_SIZEOF_${type})
      if(NOT KWSYS_SIZEOF_${type})
        set(KWSYS_SIZEOF_${type} 0)
      endif()
      set(KWSYS_C_CODE_SIZEOF_${type}
        "#define ${KWSYS_NAMESPACE}_SIZEOF_${type} ${KWSYS_SIZEOF_${type}}")
    endif()
  endforeach()

  # Check uniqueness of types.
  if(KWSYS_SIZEOF___INT64)
    KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_SAME_LONG_AND___INT64
      "Checking whether long and __int64 are the same type" DIRECT)
    if(KWSYS_SIZEOF_LONG_LONG)
      KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_SAME_LONG_LONG_AND___INT64
        "Checking whether long long and __int64 are the same type" DIRECT)
    endif(KWSYS_SIZEOF_LONG_LONG)
  endif(KWSYS_SIZEOF___INT64)

  # Enable the "long long" type if it is available.  It is standard in
  # C99 and C++03 but not in earlier standards.
  if(KWSYS_SIZEOF_LONG_LONG)
    set(KWSYS_USE_LONG_LONG 1)
  else(KWSYS_SIZEOF_LONG_LONG)
    set(KWSYS_USE_LONG_LONG 0)
  endif(KWSYS_SIZEOF_LONG_LONG)

  # Enable the "__int64" type if it is available and unique.  It is not
  # standard.
  set(KWSYS_USE___INT64 0)
  if(KWSYS_SIZEOF___INT64)
    if(NOT KWSYS_CXX_SAME_LONG_AND___INT64)
      if(NOT KWSYS_CXX_SAME_LONG_LONG_AND___INT64)
        set(KWSYS_USE___INT64 1)
      endif(NOT KWSYS_CXX_SAME_LONG_LONG_AND___INT64)
    endif(NOT KWSYS_CXX_SAME_LONG_AND___INT64)
  endif(KWSYS_SIZEOF___INT64)
  if(KWSYS_USE___INT64)
    KWSYS_PLATFORM_CXX_TEST(KWSYS_CAN_CONVERT_UI64_TO_DOUBLE
      "Checking whether unsigned __int64 can convert to double" DIRECT)
  else(KWSYS_USE___INT64)
    set(KWSYS_CAN_CONVERT_UI64_TO_DOUBLE 1)
  endif(KWSYS_USE___INT64)

  # Check signedness of "char" type.
  KWSYS_PLATFORM_CXX_TEST_RUN(KWSYS_CHAR_IS_SIGNED
    "Checking whether char is signed" DIRECT)
endif(KWSYS_USE_FundamentalType)

if(KWSYS_USE_IOStream)
  # Determine whether iostreams support long long.
  KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_LONG_LONG
    "Checking whether C++ compiler has 'long long'" DIRECT)
  if(KWSYS_CXX_HAS_LONG_LONG)
    set(KWSYS_PLATFORM_CXX_TEST_DEFINES
      -DKWSYS_IOS_USE_ANSI=${KWSYS_IOS_USE_ANSI}
      -DKWSYS_IOS_HAVE_STD=${KWSYS_IOS_HAVE_STD})
    KWSYS_PLATFORM_CXX_TEST(KWSYS_IOS_HAS_ISTREAM_LONG_LONG
      "Checking if istream supports long long" DIRECT)
    KWSYS_PLATFORM_CXX_TEST(KWSYS_IOS_HAS_OSTREAM_LONG_LONG
      "Checking if ostream supports long long" DIRECT)
    set(KWSYS_PLATFORM_CXX_TEST_DEFINES)
  else()
    set(KWSYS_IOS_HAS_ISTREAM_LONG_LONG 0)
    set(KWSYS_IOS_HAS_OSTREAM_LONG_LONG 0)
  endif()
endif(KWSYS_USE_IOStream)

if(KWSYS_NAMESPACE MATCHES "^kwsys$")
  set(KWSYS_NAME_IS_KWSYS 1)
else(KWSYS_NAMESPACE MATCHES "^kwsys$")
  set(KWSYS_NAME_IS_KWSYS 0)
endif(KWSYS_NAMESPACE MATCHES "^kwsys$")

# Choose default shared/static build if not specified.
if(KWSYS_BUILD_SHARED MATCHES "^KWSYS_BUILD_SHARED$")
  set(KWSYS_BUILD_SHARED ${BUILD_SHARED_LIBS})
endif(KWSYS_BUILD_SHARED MATCHES "^KWSYS_BUILD_SHARED$")

if(KWSYS_BUILD_SHARED)
  set(KWSYS_BUILD_SHARED 1)
  set(KWSYS_LIBRARY_TYPE SHARED)
else(KWSYS_BUILD_SHARED)
  set(KWSYS_BUILD_SHARED 0)
  set(KWSYS_LIBRARY_TYPE STATIC)
endif(KWSYS_BUILD_SHARED)

#-----------------------------------------------------------------------------
# Configure some implementation details.

KWSYS_PLATFORM_C_TEST(KWSYS_C_HAS_PTRDIFF_T
  "Checking whether C compiler has ptrdiff_t in stddef.h" DIRECT)
KWSYS_PLATFORM_C_TEST(KWSYS_C_HAS_SSIZE_T
  "Checking whether C compiler has ssize_t in unistd.h" DIRECT)
set_source_files_properties(ProcessUNIX.c System.c PROPERTIES
  COMPILE_FLAGS "-DKWSYS_C_HAS_PTRDIFF_T=${KWSYS_C_HAS_PTRDIFF_T} -DKWSYS_C_HAS_SSIZE_T=${KWSYS_C_HAS_SSIZE_T}"
  )

if(KWSYS_DO_NOT_CLEAN_PUTENV)
  # Disable cleanup of putenv memory for issues with GCOV.
  set_source_files_properties(SystemTools.cxx PROPERTIES
    COMPILE_FLAGS -DKWSYS_DO_NOT_CLEAN_PUTENV=1)
endif(KWSYS_DO_NOT_CLEAN_PUTENV)

#-----------------------------------------------------------------------------
# Choose a directory for the generated headers.
if(NOT KWSYS_HEADER_ROOT)
  set(KWSYS_HEADER_ROOT "${PROJECT_BINARY_DIR}")
endif(NOT KWSYS_HEADER_ROOT)
set(KWSYS_HEADER_DIR "${KWSYS_HEADER_ROOT}/${KWSYS_NAMESPACE}")
include_directories(${KWSYS_HEADER_ROOT})

#-----------------------------------------------------------------------------
if(KWSYS_INSTALL_DOC_DIR)
  # Assign the license to the runtime component since it must be
  # distributed with binary forms of this software.
  if(KWSYS_INSTALL_COMPONENT_NAME_RUNTIME)
    set(KWSYS_INSTALL_LICENSE_OPTIONS ${KWSYS_INSTALL_LICENSE_OPTIONS}
      COMPONENT ${KWSYS_INSTALL_COMPONENT_NAME_RUNTIME}
      )
  endif(KWSYS_INSTALL_COMPONENT_NAME_RUNTIME)

  # Install the license under the documentation directory.
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt
    DESTINATION ${KWSYS_INSTALL_DOC_DIR}/${KWSYS_NAMESPACE}
    ${KWSYS_INSTALL_LICENSE_OPTIONS})
endif(KWSYS_INSTALL_DOC_DIR)

#-----------------------------------------------------------------------------
# Create STL header wrappers to block warnings in the STL headers and
# give standard names by which they may be included.
set(KWSYS_STL_HEADER_EXTRA_string 1)
foreach(header
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
  set(KWSYS_STL_HEADER "${header}")
  if(KWSYS_STL_HEADER_EXTRA_${header})
    set(KWSYS_STL_HEADER_EXTRA
      "#define ${KWSYS_NAMESPACE}_stl_${header}_including_hxx\n# include <${KWSYS_NAMESPACE}/stl/${header}.hxx>\n#undef ${KWSYS_NAMESPACE}_stl_${header}_including_hxx\n")
    configure_file(${PROJECT_SOURCE_DIR}/kwsys_stl_${header}.hxx.in
                   ${KWSYS_HEADER_DIR}/stl/${header}.hxx
                   @ONLY IMMEDIATE)
    if(KWSYS_INSTALL_INCLUDE_DIR)
      install(FILES ${KWSYS_HEADER_DIR}/stl/${header}.hxx
        DESTINATION ${KWSYS_INSTALL_INCLUDE_DIR}/${KWSYS_NAMESPACE}/stl
        ${KWSYS_INSTALL_INCLUDE_OPTIONS})
    endif(KWSYS_INSTALL_INCLUDE_DIR)
  else(KWSYS_STL_HEADER_EXTRA_${header})
    set(KWSYS_STL_HEADER_EXTRA "")
  endif(KWSYS_STL_HEADER_EXTRA_${header})
  configure_file(${PROJECT_SOURCE_DIR}/kwsys_stl.hxx.in
                 ${KWSYS_HEADER_DIR}/stl/${header}
                 @ONLY IMMEDIATE)

  # Create an install target for the header wrapper.
  if(KWSYS_INSTALL_INCLUDE_DIR)
    install(FILES ${KWSYS_HEADER_DIR}/stl/${header}
      DESTINATION ${KWSYS_INSTALL_INCLUDE_DIR}/${KWSYS_NAMESPACE}/stl
      ${KWSYS_INSTALL_INCLUDE_OPTIONS})
  endif(KWSYS_INSTALL_INCLUDE_DIR)
endforeach(header)

# Provide cstddef header.
configure_file(${PROJECT_SOURCE_DIR}/kwsys_cstddef.hxx.in
               ${KWSYS_HEADER_DIR}/cstddef
               @ONLY IMMEDIATE)
if(KWSYS_INSTALL_INCLUDE_DIR)
  install(FILES ${KWSYS_HEADER_DIR}/cstddef
    DESTINATION ${KWSYS_INSTALL_INCLUDE_DIR}/${KWSYS_NAMESPACE}
    ${KWSYS_INSTALL_INCLUDE_OPTIONS})
endif(KWSYS_INSTALL_INCLUDE_DIR)

#-----------------------------------------------------------------------------
# Create streams header wrappers to give standard names by which they
# may be included.
foreach(header iostream fstream sstream iosfwd)
  # Configure the header wrapper.
  configure_file(${PROJECT_SOURCE_DIR}/kwsys_ios_${header}.h.in
                 ${KWSYS_HEADER_DIR}/ios/${header}
                 @ONLY IMMEDIATE)

  # Create an install target for the header wrapper.
  if(KWSYS_INSTALL_INCLUDE_DIR)
    install(FILES ${KWSYS_HEADER_DIR}/ios/${header}
      DESTINATION ${KWSYS_INSTALL_INCLUDE_DIR}/${KWSYS_NAMESPACE}/ios
      ${KWSYS_INSTALL_INCLUDE_OPTIONS})
  endif(KWSYS_INSTALL_INCLUDE_DIR)
endforeach(header)

#-----------------------------------------------------------------------------
# Build a list of classes and headers we need to implement the
# selected components.  Initialize with required components.
set(KWSYS_CLASSES)
set(KWSYS_H_FILES Configure SharedForward)
set(KWSYS_HXX_FILES Configure String
  hashtable hash_fun hash_map hash_set
  auto_ptr
  )

# Add selected C++ classes.
set(cppclasses
  Directory DynamicLoader Glob RegularExpression SystemTools
  CommandLineArguments Registry IOStream SystemInformation
  )
foreach(cpp ${cppclasses})
  if(KWSYS_USE_${cpp})
    # Use the corresponding class.
    set(KWSYS_CLASSES ${KWSYS_CLASSES} ${cpp})

    # Load component-specific CMake code.
    if(EXISTS ${PROJECT_SOURCE_DIR}/kwsys${cpp}.cmake)
      include(${PROJECT_SOURCE_DIR}/kwsys${cpp}.cmake)
    endif(EXISTS ${PROJECT_SOURCE_DIR}/kwsys${cpp}.cmake)
  endif(KWSYS_USE_${cpp})
endforeach(cpp)

# Add selected C components.
foreach(c
    Process Base64 FundamentalType MD5 Terminal System DateStamp String CPU
    )
  if(KWSYS_USE_${c})
    # Use the corresponding header file.
    set(KWSYS_H_FILES ${KWSYS_H_FILES} ${c})

    # Load component-specific CMake code.
    if(EXISTS ${PROJECT_SOURCE_DIR}/kwsys${c}.cmake)
      include(${PROJECT_SOURCE_DIR}/kwsys${c}.cmake)
    endif(EXISTS ${PROJECT_SOURCE_DIR}/kwsys${c}.cmake)
  endif(KWSYS_USE_${c})
endforeach(c)

#-----------------------------------------------------------------------------
# Build a list of sources for the library based on components that are
# included.
set(KWSYS_C_SRCS)
set(KWSYS_CXX_SRCS)

# Add the proper sources for this platform's Process implementation.
if(KWSYS_USE_Process)
  if(NOT UNIX)
    # Use the Windows implementation.  We need the encoded forwarding executable.
    set(KWSYS_C_SRCS ${KWSYS_C_SRCS} ProcessWin32.c
      ${PROJECT_BINARY_DIR}/${KWSYS_NAMESPACE}ProcessFwd9xEnc.c)
    set_source_files_properties(
      ${PROJECT_BINARY_DIR}/${KWSYS_NAMESPACE}ProcessFwd9xEnc.c
      PROPERTIES GENERATED 1)
  else(NOT UNIX)
    # Use the UNIX implementation.
    set(KWSYS_C_SRCS ${KWSYS_C_SRCS} ProcessUNIX.c)
  endif(NOT UNIX)
endif(KWSYS_USE_Process)

# Add selected C sources.
foreach(c Base64 MD5 Terminal System String)
  if(KWSYS_USE_${c})
    set(KWSYS_C_SRCS ${KWSYS_C_SRCS} ${c}.c)
  endif(KWSYS_USE_${c})
endforeach(c)

# Configure headers of C++ classes and construct the list of sources.
foreach(c ${KWSYS_CLASSES})
  # Add this source to the list of source files for the library.
  set(KWSYS_CXX_SRCS ${KWSYS_CXX_SRCS} ${c}.cxx)

  # Configure the header for this class.
  configure_file(${PROJECT_SOURCE_DIR}/${c}.hxx.in ${KWSYS_HEADER_DIR}/${c}.hxx
                 @ONLY IMMEDIATE)
  set(KWSYS_CXX_SRCS ${KWSYS_CXX_SRCS} ${KWSYS_HEADER_DIR}/${c}.hxx)

  # Create an install target for the header.
  if(KWSYS_INSTALL_INCLUDE_DIR)
    install(FILES ${KWSYS_HEADER_DIR}/${c}.hxx
      DESTINATION ${KWSYS_INSTALL_INCLUDE_DIR}/${KWSYS_NAMESPACE}
      ${KWSYS_INSTALL_INCLUDE_OPTIONS})
  endif(KWSYS_INSTALL_INCLUDE_DIR)
endforeach(c)

# Configure C headers.
foreach(h ${KWSYS_H_FILES})
  # Configure the header into the given directory.
  configure_file(${PROJECT_SOURCE_DIR}/${h}.h.in ${KWSYS_HEADER_DIR}/${h}.h
                 @ONLY IMMEDIATE)
  set(KWSYS_C_SRCS ${KWSYS_C_SRCS} ${KWSYS_HEADER_DIR}/${h}.h)

  # Create an install target for the header.
  if(KWSYS_INSTALL_INCLUDE_DIR)
    install(FILES ${KWSYS_HEADER_DIR}/${h}.h
      DESTINATION ${KWSYS_INSTALL_INCLUDE_DIR}/${KWSYS_NAMESPACE}
      ${KWSYS_INSTALL_INCLUDE_OPTIONS})
  endif(KWSYS_INSTALL_INCLUDE_DIR)
endforeach(h)

# Configure other C++ headers.
foreach(h ${KWSYS_HXX_FILES})
  # Configure the header into the given directory.
  configure_file(${PROJECT_SOURCE_DIR}/${h}.hxx.in ${KWSYS_HEADER_DIR}/${h}.hxx
                 @ONLY IMMEDIATE)
  set(KWSYS_CXX_SRCS ${KWSYS_CXX_SRCS} ${KWSYS_HEADER_DIR}/${h}.hxx)

  # Create an install target for the header.
  if(KWSYS_INSTALL_INCLUDE_DIR)
    install(FILES ${KWSYS_HEADER_DIR}/${h}.hxx
      DESTINATION ${KWSYS_INSTALL_INCLUDE_DIR}/${KWSYS_NAMESPACE}
      ${KWSYS_INSTALL_INCLUDE_OPTIONS})
  endif(KWSYS_INSTALL_INCLUDE_DIR)
endforeach(h)

#-----------------------------------------------------------------------------
# Add the library with the configured name and list of sources.
if(KWSYS_C_SRCS OR KWSYS_CXX_SRCS)
  add_library(${KWSYS_NAMESPACE} ${KWSYS_LIBRARY_TYPE}
    ${KWSYS_C_SRCS} ${KWSYS_CXX_SRCS})
  set_property(TARGET ${KWSYS_NAMESPACE} PROPERTY LABELS ${KWSYS_LABELS_LIB})
  if(KWSYS_USE_DynamicLoader)
    if(UNIX)
      target_link_libraries(${KWSYS_NAMESPACE} ${CMAKE_DL_LIBS})
    endif(UNIX)
  endif(KWSYS_USE_DynamicLoader)

  if(KWSYS_USE_SystemInformation AND WIN32)
    target_link_libraries(${KWSYS_NAMESPACE} ws2_32)
  endif(KWSYS_USE_SystemInformation AND WIN32)

  # Apply user-defined target properties to the library.
  if(KWSYS_PROPERTIES_CXX)
    set_target_properties(${KWSYS_NAMESPACE} PROPERTIES
      ${KWSYS_PROPERTIES_CXX}
      )
  endif(KWSYS_PROPERTIES_CXX)

  # Create an install target for the library.
  if(KWSYS_INSTALL_LIBRARY_RULE)
    install(TARGETS ${KWSYS_NAMESPACE} ${KWSYS_INSTALL_LIBRARY_RULE})
  endif(KWSYS_INSTALL_LIBRARY_RULE)
endif(KWSYS_C_SRCS OR KWSYS_CXX_SRCS)

# Add a C-only library if requested.
if(KWSYS_ENABLE_C AND KWSYS_C_SRCS)
  add_library(${KWSYS_NAMESPACE}_c ${KWSYS_LIBRARY_TYPE} ${KWSYS_C_SRCS})
  set_property(TARGET ${KWSYS_NAMESPACE}_c PROPERTY LABELS ${KWSYS_LABELS_LIB})

  # Apply user-defined target properties to the library.
  if(KWSYS_PROPERTIES_C)
    set_target_properties(${KWSYS_NAMESPACE} PROPERTIES
      ${KWSYS_PROPERTIES_C}
      )
  endif(KWSYS_PROPERTIES_C)

  # Create an install target for the library.
  if(KWSYS_INSTALL_LIBRARY_RULE)
    install(TARGETS ${KWSYS_NAMESPACE}_c ${KWSYS_INSTALL_LIBRARY_RULE})
  endif(KWSYS_INSTALL_LIBRARY_RULE)
endif(KWSYS_ENABLE_C AND KWSYS_C_SRCS)

# For building kwsys itself, we use a macro defined on the command
# line to configure the namespace in the C and C++ source files.
add_definitions("-DKWSYS_NAMESPACE=${KWSYS_NAMESPACE}")

if(KWSYS_USE_String)
  # Activate code in "String.c".  See the comment in the source.
  set_source_files_properties(String.c PROPERTIES
    COMPILE_FLAGS "-DKWSYS_STRING_C")
endif(KWSYS_USE_String)

#-----------------------------------------------------------------------------
# Process execution on windows needs to build a forwarding executable
# that works around a Win9x bug.  We encode the executable into a C
# file and build it into the library.  Win9x platforms reproduce the
# executable into a temporary directory when it is needed.
if(KWSYS_USE_Process)
  if(NOT UNIX)
    # Build the forwarding executable itself and a program that will
    # encode it into a C file.
    add_executable(${KWSYS_NAMESPACE}ProcessFwd9x ProcessFwd9x.c)
    add_executable(${KWSYS_NAMESPACE}EncodeExecutable EncodeExecutable.c)
    set_property(TARGET ${KWSYS_NAMESPACE}ProcessFwd9x PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_property(TARGET ${KWSYS_NAMESPACE}EncodeExecutable PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_property(TARGET ${KWSYS_NAMESPACE}ProcessFwd9x PROPERTY LABELS ${KWSYS_LABELS_EXE})
    set_property(TARGET ${KWSYS_NAMESPACE}EncodeExecutable PROPERTY LABELS ${KWSYS_LABELS_EXE})

    set(CFG_INTDIR "/${CMAKE_CFG_INTDIR}")
    if(CMAKE_BUILD_TOOL MATCHES "make")
      set(CFG_INTDIR "")
    endif(CMAKE_BUILD_TOOL MATCHES "make")

    # Take advantage of a better custom command syntax if possible.
    set(CMD ${CMAKE_CURRENT_BINARY_DIR}${CFG_INTDIR}/${KWSYS_NAMESPACE}EncodeExecutable.exe)
    set(FWD ${CMAKE_CURRENT_BINARY_DIR}${CFG_INTDIR}/${KWSYS_NAMESPACE}ProcessFwd9x.exe)
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${KWSYS_NAMESPACE}ProcessFwd9xEnc.c
      COMMAND ${CMD}
      ARGS ${FWD} ${CMAKE_CURRENT_BINARY_DIR}/${KWSYS_NAMESPACE}ProcessFwd9xEnc.c
           ${KWSYS_NAMESPACE} ProcessFwd9x
      DEPENDS ${CMD} ${FWD})

    # Make sure build occurs in proper order.
    add_dependencies(${KWSYS_NAMESPACE} ${KWSYS_NAMESPACE}ProcessFwd9x
                     ${KWSYS_NAMESPACE}EncodeExecutable)
  endif(NOT UNIX)
endif(KWSYS_USE_Process)

#-----------------------------------------------------------------------------
# Setup testing if not being built as part of another project.
if(KWSYS_STANDALONE OR CMake_SOURCE_DIR)
  if(BUILD_TESTING)
    # Compute the location of executables.
    set(EXEC_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    if(EXECUTABLE_OUTPUT_PATH)
      set(EXEC_DIR "${EXECUTABLE_OUTPUT_PATH}")
    endif(EXECUTABLE_OUTPUT_PATH)

    # C tests
    set(KWSYS_C_TESTS
      testEncode
      testTerminal
      )
    if(KWSYS_STANDALONE)
      set(KWSYS_C_TESTS ${KWSYS_C_TESTS} testFail)
    endif()
    create_test_sourcelist(
      KWSYS_C_TEST_SRCS ${KWSYS_NAMESPACE}TestsC.c
      ${KWSYS_C_TESTS}
      )
    add_executable(${KWSYS_NAMESPACE}TestsC ${KWSYS_C_TEST_SRCS})
    set_property(TARGET ${KWSYS_NAMESPACE}TestsC PROPERTY LABELS ${KWSYS_LABELS_EXE})
    target_link_libraries(${KWSYS_NAMESPACE}TestsC ${KWSYS_NAMESPACE}_c)
    foreach(test ${KWSYS_C_TESTS})
      add_test(kwsys.${test} ${EXEC_DIR}/${KWSYS_NAMESPACE}TestsC ${test} ${KWSYS_TEST_ARGS_${test}})
      set_property(TEST kwsys.${test} PROPERTY LABELS ${KWSYS_LABELS_TEST})
    endforeach(test)

    # C++ tests
    if(NOT WATCOM)
      set(KWSYS_CXX_TESTS
        testAutoPtr
        testHashSTL
        )
    endif(NOT WATCOM)
    set(KWSYS_CXX_TESTS ${KWSYS_CXX_TESTS}
      testRegistry
      testIOS
      testSystemTools
      testCommandLineArguments
      testCommandLineArguments1
      )
    if(KWSYS_USE_SystemInformation)
      set(KWSYS_CXX_TESTS ${KWSYS_CXX_TESTS} testSystemInformation)
    endif(KWSYS_USE_SystemInformation)
    if(KWSYS_USE_DynamicLoader)
      set(KWSYS_CXX_TESTS ${KWSYS_CXX_TESTS} testDynamicLoader)
      # If kwsys contains the DynamicLoader, need extra library
      add_library(${KWSYS_NAMESPACE}TestDynload MODULE testDynload.c)
      set_property(TARGET ${KWSYS_NAMESPACE}TestDynload PROPERTY LABELS ${KWSYS_LABELS_LIB})
      add_dependencies(${KWSYS_NAMESPACE}TestDynload ${KWSYS_NAMESPACE})
    endif(KWSYS_USE_DynamicLoader)
    create_test_sourcelist(
      KWSYS_CXX_TEST_SRCS ${KWSYS_NAMESPACE}TestsCxx.cxx
      ${KWSYS_CXX_TESTS}
      )
    add_executable(${KWSYS_NAMESPACE}TestsCxx ${KWSYS_CXX_TEST_SRCS})
    set_property(TARGET ${KWSYS_NAMESPACE}TestsCxx PROPERTY LABELS ${KWSYS_LABELS_EXE})
    target_link_libraries(${KWSYS_NAMESPACE}TestsCxx ${KWSYS_NAMESPACE})
    set(TEST_SYSTEMTOOLS_BIN_FILE
      "${CMAKE_CURRENT_SOURCE_DIR}/testSystemTools.bin")
    set(TEST_SYSTEMTOOLS_SRC_FILE
      "${CMAKE_CURRENT_SOURCE_DIR}/testSystemTools.cxx")
    configure_file(
      ${PROJECT_SOURCE_DIR}/testSystemTools.h.in
      ${PROJECT_BINARY_DIR}/testSystemTools.h)
    include_directories(${PROJECT_BINARY_DIR})

    if(CTEST_TEST_KWSYS)
      configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ExtraTest.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ExtraTest.cmake")
      set_directory_properties(PROPERTIES TEST_INCLUDE_FILE "${CMAKE_CURRENT_BINARY_DIR}/ExtraTest.cmake")
    endif(CTEST_TEST_KWSYS)

    set(KWSYS_TEST_ARGS_testCommandLineArguments
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
    set(KWSYS_TEST_ARGS_testCommandLineArguments1
      --ignored
      -n 24
      --second-ignored
      "-m=test value"
      third-ignored
      -p
      some junk at the end
      )
    foreach(test ${KWSYS_CXX_TESTS})
      add_test(kwsys.${test} ${EXEC_DIR}/${KWSYS_NAMESPACE}TestsCxx ${test} ${KWSYS_TEST_ARGS_${test}})
      set_property(TEST kwsys.${test} PROPERTY LABELS ${KWSYS_LABELS_TEST})
    endforeach(test)

    # Process tests.
    add_executable(${KWSYS_NAMESPACE}TestProcess testProcess.c)
    set_property(TARGET ${KWSYS_NAMESPACE}TestProcess PROPERTY LABELS ${KWSYS_LABELS_EXE})
    target_link_libraries(${KWSYS_NAMESPACE}TestProcess ${KWSYS_NAMESPACE}_c)
    if(NOT CYGWIN)
      set(KWSYS_TEST_PROCESS_7 7)
    endif(NOT CYGWIN)
    foreach(n 1 2 3 4 5 6 ${KWSYS_TEST_PROCESS_7})
      add_test(kwsys.testProcess-${n} ${EXEC_DIR}/${KWSYS_NAMESPACE}TestProcess ${n})
      set_property(TEST kwsys.testProcess-${n} PROPERTY LABELS ${KWSYS_LABELS_TEST})
      set_tests_properties(kwsys.testProcess-${n} PROPERTIES TIMEOUT 120)
    endforeach(n)

    # Some Apple compilers produce bad optimizations in this source.
    if(APPLE AND "${CMAKE_C_COMPILER_ID}" MATCHES "^(GNU|LLVM)$")
      set_source_files_properties(testProcess.c PROPERTIES COMPILE_FLAGS -O0)
    endif()

    # Test SharedForward
    configure_file(${PROJECT_SOURCE_DIR}/testSharedForward.c.in
                   ${PROJECT_BINARY_DIR}/testSharedForward.c @ONLY IMMEDIATE)
    add_executable(${KWSYS_NAMESPACE}TestSharedForward
                   ${PROJECT_BINARY_DIR}/testSharedForward.c)
    set_property(TARGET ${KWSYS_NAMESPACE}TestSharedForward PROPERTY LABELS ${KWSYS_LABELS_EXE})
    add_dependencies(${KWSYS_NAMESPACE}TestSharedForward ${KWSYS_NAMESPACE}_c)
    add_test(kwsys.testSharedForward ${EXEC_DIR}/${KWSYS_NAMESPACE}TestSharedForward 1)
    set_property(TEST kwsys.testSharedForward PROPERTY LABELS ${KWSYS_LABELS_TEST})

    # Configure some test properties.
    if(KWSYS_STANDALONE)
      # We expect test to fail
      set_tests_properties(kwsys.testFail PROPERTIES WILL_FAIL ON)
      get_test_property(kwsys.testFail WILL_FAIL wfv)
      set_tests_properties(kwsys.testRegistry PROPERTIES FAIL_REGULAR_EXPRESSION "ERROR_NOT_VALGRIND;FAIL;Test failed")
      set_tests_properties(kwsys.testRegistry PROPERTIES PASS_REGULAR_EXPRESSION "Test passed")
      set_tests_properties(kwsys.testFail PROPERTIES MEASUREMENT "Some Key=Some Value")
      message(STATUS "GET_TEST_PROPERTY returned: ${wfv}")
    endif()

    # Suppress known consistent failures on buggy systems.
    if(KWSYS_TEST_BOGUS_FAILURES)
      set_tests_properties(${KWSYS_TEST_BOGUS_FAILURES} PROPERTIES WILL_FAIL ON)
    endif()

  endif(BUILD_TESTING)
endif(KWSYS_STANDALONE OR CMake_SOURCE_DIR)
