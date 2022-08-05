# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing#kwsys for details.

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
#  add_subdirectory(kwsys)
#
# Optional settings are as follows:
#
#  KWSYS_HEADER_ROOT = The directory into which to generate the kwsys headers.
#                      A directory called "${KWSYS_NAMESPACE}" will be
#                      created under this root directory to hold the files.
#  KWSYS_SPLIT_OBJECTS_FROM_INTERFACE
#                    = Instead of creating a single ${KWSYS_NAMESPACE} library
#                      target, create three separate targets:
#                        ${KWSYS_NAMESPACE}
#                          - An INTERFACE library only containing usage
#                            requirements.
#                        ${KWSYS_NAMESPACE}_objects
#                          - An OBJECT library for the built kwsys objects.
#                            Note: This is omitted from the install rules
#                        ${KWSYS_NAMESPACE}_private
#                          - An INTERFACE library combining both that is
#                            appropriate for use with PRIVATE linking in
#                            target_link_libraries. Because of how interface
#                            properties propagate, this target is not suitable
#                            for use with PUBLIC or INTERFACE linking.
#  KWSYS_ALIAS_TARGET = The name of an alias target to create to the actual target.
#
#    Example:
#
#      set(KWSYS_HEADER_ROOT ${PROJECT_BINARY_DIR})
#      include_directories(${PROJECT_BINARY_DIR})
#
#  KWSYS_CXX_STANDARD         = A value for CMAKE_CXX_STANDARD within KWSys.
#                               Set to empty string to use no default value.
#  KWSYS_CXX_COMPILE_FEATURES = target_compile_features arguments for KWSys.
#
#  KWSYS_NO_EXECINFO          = Do not use execinfo.
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

cmake_minimum_required(VERSION 3.9...3.22 FATAL_ERROR)

# Some configure checks depend upon the deployment target. Clear checks when
# the deployment target changes.
if (APPLE)
  if (NOT CMAKE_OSX_DEPLOYMENT_TARGET STREQUAL KWSYS_LAST_OSX_DEPLOYMENT_TARGET)
    unset(KWSYS_CXX_HAS_UTIMENSAT CACHE)
  endif ()
  set(KWSYS_LAST_OSX_DEPLOYMENT_TARGET "${CMAKE_OSX_DEPLOYMENT_TARGET}"
    CACHE INTERNAL "remember the last deployment target to trigger configure rechecks")
endif ()

#-----------------------------------------------------------------------------
# If a namespace is not specified, use "kwsys" and enable testing.
# This should be the case only when kwsys is not included inside
# another project and is being tested.
if(NOT KWSYS_NAMESPACE)
  set(KWSYS_NAMESPACE "kwsys")
  set(KWSYS_STANDALONE 1)
endif()

#-----------------------------------------------------------------------------
# The project name is that of the specified namespace.
project(${KWSYS_NAMESPACE})

# Tell CMake how to follow dependencies of sources in this directory.
set_property(DIRECTORY
  PROPERTY IMPLICIT_DEPENDS_INCLUDE_TRANSFORM
  "KWSYS_HEADER(%)=<${KWSYS_NAMESPACE}/%>"
  )

if(KWSYS_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD "${KWSYS_CXX_STANDARD}")
elseif(NOT DEFINED CMAKE_CXX_STANDARD AND NOT DEFINED KWSYS_CXX_STANDARD)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
     AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"
     AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU"
    )
    set(CMAKE_CXX_STANDARD 14)
  else()
    set(CMAKE_CXX_STANDARD 11)
  endif()
endif()

# Select library components.
if(KWSYS_STANDALONE OR CMake_SOURCE_DIR)
  set(KWSYS_ENABLE_C 1)
  # Enable all components.
  set(KWSYS_USE_Base64 1)
  set(KWSYS_USE_Directory 1)
  set(KWSYS_USE_DynamicLoader 1)
  set(KWSYS_USE_Encoding 1)
  set(KWSYS_USE_Glob 1)
  set(KWSYS_USE_MD5 1)
  set(KWSYS_USE_Process 1)
  set(KWSYS_USE_RegularExpression 1)
  set(KWSYS_USE_Status 1)
  set(KWSYS_USE_System 1)
  set(KWSYS_USE_SystemTools 1)
  set(KWSYS_USE_CommandLineArguments 1)
  set(KWSYS_USE_Terminal 1)
  set(KWSYS_USE_FStream 1)
  set(KWSYS_USE_String 1)
  set(KWSYS_USE_SystemInformation 1)
  set(KWSYS_USE_ConsoleBuf 1)
endif()

# Enforce component dependencies.
if(KWSYS_USE_SystemTools)
  set(KWSYS_USE_Directory 1)
  set(KWSYS_USE_FStream 1)
  set(KWSYS_USE_Encoding 1)
  set(KWSYS_USE_Status 1)
endif()
if(KWSYS_USE_Glob)
  set(KWSYS_USE_Directory 1)
  set(KWSYS_USE_SystemTools 1)
  set(KWSYS_USE_RegularExpression 1)
  set(KWSYS_USE_FStream 1)
  set(KWSYS_USE_Encoding 1)
endif()
if(KWSYS_USE_Process)
  set(KWSYS_USE_System 1)
  set(KWSYS_USE_Encoding 1)
endif()
if(KWSYS_USE_SystemInformation)
  set(KWSYS_USE_Process 1)
endif()
if(KWSYS_USE_System)
  set(KWSYS_USE_Encoding 1)
endif()
if(KWSYS_USE_Directory)
  set(KWSYS_USE_Encoding 1)
  set(KWSYS_USE_Status 1)
  set(KWSYS_USE_SystemTools 1)
endif()
if(KWSYS_USE_DynamicLoader)
  set(KWSYS_USE_Encoding 1)
endif()
if(KWSYS_USE_FStream)
  set(KWSYS_USE_Encoding 1)
endif()
if(KWSYS_USE_ConsoleBuf)
  set(KWSYS_USE_Encoding 1)
endif()

# Specify default 8 bit encoding for Windows
if(NOT KWSYS_ENCODING_DEFAULT_CODEPAGE)
  set(KWSYS_ENCODING_DEFAULT_CODEPAGE CP_ACP)
endif()

# Enable testing if building standalone.
if(KWSYS_STANDALONE)
  include(Dart)
  mark_as_advanced(BUILD_TESTING DART_ROOT TCL_TCLSH)
  if(BUILD_TESTING)
    enable_testing()
  endif()
endif()

# Choose default shared/static build if not specified.
if(NOT DEFINED KWSYS_BUILD_SHARED)
  set(KWSYS_BUILD_SHARED ${BUILD_SHARED_LIBS})
endif()

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
endif()
if(NOT KWSYS_INSTALL_LIB_DIR)
  string(REGEX REPLACE "^/" "" KWSYS_INSTALL_LIB_DIR
    "${KWSYS_LIBRARY_INSTALL_DIR}")
endif()
if(NOT KWSYS_INSTALL_BIN_DIR)
  string(REGEX REPLACE "^/" "" KWSYS_INSTALL_BIN_DIR
    "${KWSYS_LIBRARY_INSTALL_DIR}")
endif()

# Setup header install rules.
set(KWSYS_INSTALL_INCLUDE_OPTIONS)
if(KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT)
  set(KWSYS_INSTALL_INCLUDE_OPTIONS ${KWSYS_INSTALL_INCLUDE_OPTIONS}
    COMPONENT ${KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT}
    )
endif()

# Setup library install rules.
set(KWSYS_INSTALL_LIBRARY_RULE)
set(KWSYS_INSTALL_NAMELINK_RULE)
if(KWSYS_INSTALL_LIB_DIR)
  if(KWSYS_INSTALL_EXPORT_NAME)
    list(APPEND KWSYS_INSTALL_LIBRARY_RULE EXPORT ${KWSYS_INSTALL_EXPORT_NAME})
  endif()
  # Install the shared library to the lib directory.
  set(KWSYS_INSTALL_LIBRARY_RULE ${KWSYS_INSTALL_LIBRARY_RULE}
    LIBRARY DESTINATION ${KWSYS_INSTALL_LIB_DIR} NAMELINK_SKIP
    )
  # Assign the shared library to the runtime component.
  if(KWSYS_INSTALL_COMPONENT_NAME_RUNTIME)
    set(KWSYS_INSTALL_LIBRARY_RULE ${KWSYS_INSTALL_LIBRARY_RULE}
      COMPONENT ${KWSYS_INSTALL_COMPONENT_NAME_RUNTIME}
      )
  endif()
  if(KWSYS_BUILD_SHARED)
    set(KWSYS_INSTALL_NAMELINK_RULE ${KWSYS_INSTALL_NAMELINK_RULE}
      LIBRARY DESTINATION ${KWSYS_INSTALL_LIB_DIR} NAMELINK_ONLY
      )
    # Assign the namelink to the development component.
    if(KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT)
      set(KWSYS_INSTALL_NAMELINK_RULE ${KWSYS_INSTALL_NAMELINK_RULE}
        COMPONENT ${KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT}
        )
    endif()
  endif()

  # Install the archive to the lib directory.
  set(KWSYS_INSTALL_LIBRARY_RULE ${KWSYS_INSTALL_LIBRARY_RULE}
    ARCHIVE DESTINATION ${KWSYS_INSTALL_LIB_DIR}
    )
  # Assign the archive to the development component.
  if(KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT)
    set(KWSYS_INSTALL_LIBRARY_RULE ${KWSYS_INSTALL_LIBRARY_RULE}
      COMPONENT ${KWSYS_INSTALL_COMPONENT_NAME_DEVELOPMENT}
      )
  endif()
endif()
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
  endif()
endif()

# Do not support old KWSYS_*a_INSTALL_DIR variable names.
set(KWSYS_HEADER_INSTALL_DIR)
set(KWSYS_LIBRARY_INSTALL_DIR)

# Generated source files will need this header.
string(COMPARE EQUAL "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}"
  KWSYS_IN_SOURCE_BUILD)
if(NOT KWSYS_IN_SOURCE_BUILD)
  configure_file(${PROJECT_SOURCE_DIR}/kwsysPrivate.h
    ${PROJECT_BINARY_DIR}/kwsysPrivate.h COPYONLY IMMEDIATE)
endif()

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
endif()

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
    endif()
    if(NOT KWSYS_CXX_FLAGS_HAVE_NO_IMPLICIT_INCLUDE)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -no_implicit_include")
    endif()
  endif()
  if(CMAKE_SYSTEM MATCHES "HP-UX")
    set(KWSYS_PLATFORM_CXX_TEST_EXTRA_FLAGS "+p")
    if(CMAKE_CXX_COMPILER_ID MATCHES "HP")
      # it is known that version 3.85 fails and 6.25 works without these flags
      if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4)
        # use new C++ library and improved template support
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -AA +hpxstd98")
      endif()
    endif()
  endif()
endif()
if(KWSYS_STANDALONE)
  if(CMAKE_CXX_COMPILER_ID STREQUAL SunPro)
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.13)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++03")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -library=stlport4")
    endif()
  endif()
endif()

#-----------------------------------------------------------------------------
# Configure the standard library header wrappers based on compiler's
# capabilities and parent project's request.  Enforce 0/1 as only
# possible values for configuration into Configure.hxx.

if(KWSYS_USE_Encoding)
  # Look for type size helper macros.
  KWSYS_PLATFORM_CXX_TEST(KWSYS_STL_HAS_WSTRING
    "Checking whether wstring is available" DIRECT)
endif()

if(KWSYS_NAMESPACE MATCHES "^kwsys$")
  set(KWSYS_NAME_IS_KWSYS 1)
else()
  set(KWSYS_NAME_IS_KWSYS 0)
endif()

if(KWSYS_BUILD_SHARED)
  set(KWSYS_BUILD_SHARED 1)
  set(KWSYS_LIBRARY_TYPE SHARED)
else()
  set(KWSYS_BUILD_SHARED 0)
  set(KWSYS_LIBRARY_TYPE STATIC)
endif()

if(NOT DEFINED KWSYS_BUILD_PIC)
  set(KWSYS_BUILD_PIC 0)
endif()

#-----------------------------------------------------------------------------
# Configure some implementation details.

KWSYS_PLATFORM_C_TEST(KWSYS_C_HAS_PTRDIFF_T
  "Checking whether C compiler has ptrdiff_t in stddef.h" DIRECT)
KWSYS_PLATFORM_C_TEST(KWSYS_C_HAS_SSIZE_T
  "Checking whether C compiler has ssize_t in unistd.h" DIRECT)
if(KWSYS_USE_Process)
  KWSYS_PLATFORM_C_TEST(KWSYS_C_HAS_CLOCK_GETTIME_MONOTONIC
    "Checking whether C compiler has clock_gettime" DIRECT)
endif()

set_source_files_properties(ProcessUNIX.c System.c PROPERTIES
  COMPILE_FLAGS "-DKWSYS_C_HAS_PTRDIFF_T=${KWSYS_C_HAS_PTRDIFF_T} -DKWSYS_C_HAS_SSIZE_T=${KWSYS_C_HAS_SSIZE_T} -DKWSYS_C_HAS_CLOCK_GETTIME_MONOTONIC=${KWSYS_C_HAS_CLOCK_GETTIME_MONOTONIC}"
  )

if(DEFINED KWSYS_PROCESS_USE_SELECT)
  get_property(ProcessUNIX_FLAGS SOURCE ProcessUNIX.c PROPERTY COMPILE_FLAGS)
  set_property(SOURCE ProcessUNIX.c PROPERTY COMPILE_FLAGS "${ProcessUNIX_FLAGS} -DKWSYSPE_USE_SELECT=${KWSYSPE_USE_SELECT}")
endif()

if(KWSYS_USE_DynamicLoader)
  get_property(KWSYS_SUPPORTS_SHARED_LIBS GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)
  if(KWSYS_SUPPORTS_SHARED_LIBS)
    set(KWSYS_SUPPORTS_SHARED_LIBS 1)
  else()
    set(KWSYS_SUPPORTS_SHARED_LIBS 0)
  endif()
  set_property(SOURCE DynamicLoader.cxx APPEND PROPERTY COMPILE_DEFINITIONS
    KWSYS_SUPPORTS_SHARED_LIBS=${KWSYS_SUPPORTS_SHARED_LIBS})
endif()

if(KWSYS_USE_SystemTools)
  if (NOT DEFINED KWSYS_SYSTEMTOOLS_USE_TRANSLATION_MAP)
    set(KWSYS_SYSTEMTOOLS_USE_TRANSLATION_MAP 1)
  endif ()
  if (KWSYS_SYSTEMTOOLS_USE_TRANSLATION_MAP)
    set(KWSYS_SYSTEMTOOLS_USE_TRANSLATION_MAP 1)
  else ()
    set(KWSYS_SYSTEMTOOLS_USE_TRANSLATION_MAP 0)
  endif ()
  KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_SETENV
    "Checking whether CXX compiler has setenv" DIRECT)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_UNSETENV
    "Checking whether CXX compiler has unsetenv" DIRECT)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_ENVIRON_IN_STDLIB_H
    "Checking whether CXX compiler has environ in stdlib.h" DIRECT)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_UTIMES
    "Checking whether CXX compiler has utimes" DIRECT)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_UTIMENSAT
    "Checking whether CXX compiler has utimensat" DIRECT)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_STAT_HAS_ST_MTIM
    "Checking whether CXX compiler struct stat has st_mtim member" DIRECT)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_STAT_HAS_ST_MTIMESPEC
    "Checking whether CXX compiler struct stat has st_mtimespec member" DIRECT)
  set_property(SOURCE SystemTools.cxx APPEND PROPERTY COMPILE_DEFINITIONS
    KWSYS_CXX_HAS_SETENV=${KWSYS_CXX_HAS_SETENV}
    KWSYS_CXX_HAS_UNSETENV=${KWSYS_CXX_HAS_UNSETENV}
    KWSYS_CXX_HAS_ENVIRON_IN_STDLIB_H=${KWSYS_CXX_HAS_ENVIRON_IN_STDLIB_H}
    KWSYS_CXX_HAS_UTIMES=${KWSYS_CXX_HAS_UTIMES}
    KWSYS_CXX_HAS_UTIMENSAT=${KWSYS_CXX_HAS_UTIMENSAT}
    KWSYS_CXX_STAT_HAS_ST_MTIM=${KWSYS_CXX_STAT_HAS_ST_MTIM}
    KWSYS_CXX_STAT_HAS_ST_MTIMESPEC=${KWSYS_CXX_STAT_HAS_ST_MTIMESPEC}
    )
  if(NOT WIN32)
    if(KWSYS_STANDALONE)
      option(KWSYS_SYSTEMTOOLS_SUPPORT_WINDOWS_SLASHES "If true, Windows paths will be supported on Unix as well" ON)
    endif()
    if(KWSYS_SYSTEMTOOLS_SUPPORT_WINDOWS_SLASHES)
      set_property(SOURCE SystemTools.cxx testSystemTools.cxx APPEND PROPERTY COMPILE_DEFINITIONS
        KWSYS_SYSTEMTOOLS_SUPPORT_WINDOWS_SLASHES
        )
    endif()
  endif()

  # Disable getpwnam for static linux builds since it depends on shared glibc
  get_property(SHARED_LIBS_SUPPORTED GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)
  if(CMAKE_SYSTEM_NAME MATCHES "Linux" AND NOT SHARED_LIBS_SUPPORTED)
    set_property(SOURCE SystemTools.cxx APPEND PROPERTY COMPILE_DEFINITIONS
      HAVE_GETPWNAM=0
      )
  endif()
endif()

if(KWSYS_USE_SystemInformation)
  set_property(SOURCE SystemInformation.cxx APPEND PROPERTY
    COMPILE_DEFINITIONS SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P})
  if(NOT CYGWIN)
    include(CheckIncludeFiles)
    CHECK_INCLUDE_FILES("sys/types.h;ifaddrs.h" KWSYS_SYS_HAS_IFADDRS_H)
    if(KWSYS_SYS_HAS_IFADDRS_H)
      set_property(SOURCE SystemInformation.cxx APPEND PROPERTY
        COMPILE_DEFINITIONS KWSYS_SYS_HAS_IFADDRS_H=1)
    endif()
  endif()
  if(WIN32)
    include(CheckSymbolExists)
    set(CMAKE_REQUIRED_LIBRARIES psapi)
    CHECK_SYMBOL_EXISTS(GetProcessMemoryInfo "windows.h;psapi.h" KWSYS_SYS_HAS_PSAPI)
    unset(CMAKE_REQUIRED_LIBRARIES)
    if(KWSYS_SYS_HAS_PSAPI)
      set_property(SOURCE SystemInformation.cxx APPEND PROPERTY
        COMPILE_DEFINITIONS KWSYS_SYS_HAS_PSAPI=1)
      if(MSVC70 OR MSVC71)
        # Suppress LNK4089: all references to 'PSAPI.DLL' discarded by /OPT:REF
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /IGNORE:4089")
      endif()
    endif()
  endif()
  if(CMAKE_SYSTEM MATCHES "HP-UX")
    CHECK_INCLUDE_FILES("sys/mpctl.h" KWSYS_SYS_HAS_MPCTL_H)
    if(KWSYS_SYS_HAS_MPCTL_H)
      set_property(SOURCE SystemInformation.cxx APPEND PROPERTY
        COMPILE_DEFINITIONS KWSYS_SYS_HAS_MPCTL_H=1)
    endif()
  endif()
  if(CMAKE_SYSTEM MATCHES "BSD")
    CHECK_INCLUDE_FILES("machine/cpu.h" KWSYS_SYS_HAS_MACHINE_CPU_H)
    if(KWSYS_SYS_HAS_MACHINE_CPU_H)
      set_property(SOURCE SystemInformation.cxx APPEND PROPERTY
        COMPILE_DEFINITIONS KWSYS_SYS_HAS_MACHINE_CPU_H=1)
    endif()
  endif()
  KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_RLIMIT64
    "Checking whether CXX compiler has rlimit64" DIRECT)
  set(KWSYS_PLATFORM_CXX_TEST_DEFINES)
  if(KWSYS_CXX_HAS_RLIMIT64)
    set_property(SOURCE SystemInformation.cxx APPEND PROPERTY
      COMPILE_DEFINITIONS KWSYS_CXX_HAS_RLIMIT64=1)
  endif()
  if(UNIX AND NOT KWSYS_NO_EXECINFO)
    include(CheckIncludeFileCXX)
    # check for simple stack trace
    # usually it's in libc but on FreeBSD
    # it's in libexecinfo
    find_library(EXECINFO_LIB "execinfo")
    mark_as_advanced(EXECINFO_LIB)
    if (NOT EXECINFO_LIB)
      set(EXECINFO_LIB "")
    endif()
    CHECK_INCLUDE_FILE_CXX("execinfo.h" KWSYS_CXX_HAS_EXECINFOH)
    if (KWSYS_CXX_HAS_EXECINFOH)
      # we have the backtrace header check if it
      # can be used with this compiler
      set(KWSYS_PLATFORM_CXX_TEST_LINK_LIBRARIES ${EXECINFO_LIB})
      KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_BACKTRACE
         "Checking whether backtrace works with this C++ compiler" DIRECT)
      set(KWSYS_PLATFORM_CXX_TEST_LINK_LIBRARIES)
      if (KWSYS_CXX_HAS_BACKTRACE)
        # backtrace is supported by this system and compiler.
        # now check for the more advanced capabilities.
        set_property(SOURCE SystemInformation.cxx APPEND PROPERTY
          COMPILE_DEFINITIONS KWSYS_SYSTEMINFORMATION_HAS_BACKTRACE=1)
        # check for symbol lookup using dladdr
        CHECK_INCLUDE_FILE_CXX("dlfcn.h" KWSYS_CXX_HAS_DLFCNH)
        if (KWSYS_CXX_HAS_DLFCNH)
          # we have symbol lookup libraries and headers
          # check if they can be used with this compiler
          set(KWSYS_PLATFORM_CXX_TEST_LINK_LIBRARIES ${CMAKE_DL_LIBS})
            KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_DLADDR
            "Checking whether dladdr works with this C++ compiler" DIRECT)
          set(KWSYS_PLATFORM_CXX_TEST_LINK_LIBRARIES)
          if (KWSYS_CXX_HAS_DLADDR)
            # symbol lookup is supported by this system
            # and compiler.
            set_property(SOURCE SystemInformation.cxx APPEND PROPERTY
              COMPILE_DEFINITIONS KWSYS_SYSTEMINFORMATION_HAS_SYMBOL_LOOKUP=1)
          endif()
        endif()
        # c++ demangling support
        # check for cxxabi headers
        CHECK_INCLUDE_FILE_CXX("cxxabi.h" KWSYS_CXX_HAS_CXXABIH)
        if (KWSYS_CXX_HAS_CXXABIH)
          # check if cxxabi can be used with this
          # system and compiler.
          KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_CXXABI
            "Checking whether cxxabi works with this C++ compiler" DIRECT)
          if (KWSYS_CXX_HAS_CXXABI)
            # c++ demangle using cxxabi is supported with
            # this system and compiler
            set_property(SOURCE SystemInformation.cxx APPEND PROPERTY
              COMPILE_DEFINITIONS KWSYS_SYSTEMINFORMATION_HAS_CPP_DEMANGLE=1)
          endif()
        endif()
        # basic backtrace works better with release build
        # don't bother with advanced features for release
        set_property(SOURCE SystemInformation.cxx APPEND PROPERTY
          COMPILE_DEFINITIONS_DEBUG KWSYS_SYSTEMINFORMATION_HAS_DEBUG_BUILD=1)
        set_property(SOURCE SystemInformation.cxx APPEND PROPERTY
          COMPILE_DEFINITIONS_RELWITHDEBINFO KWSYS_SYSTEMINFORMATION_HAS_DEBUG_BUILD=1)
      endif()
    endif()
  endif()
  if(KWSYS_BUILD_SHARED)
    set_property(SOURCE SystemInformation.cxx APPEND PROPERTY
      COMPILE_DEFINITIONS KWSYS_BUILD_SHARED=1)
  endif()

  if(UNIX AND NOT CYGWIN)
    KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_GETLOADAVG
      "Checking whether CXX compiler has getloadavg" DIRECT)
    if(KWSYS_CXX_HAS_GETLOADAVG)
      set_property(SOURCE SystemInformation.cxx APPEND PROPERTY
        COMPILE_DEFINITIONS KWSYS_CXX_HAS_GETLOADAVG=1)
    endif()
  endif()
endif()

if(KWSYS_USE_FStream)
  KWSYS_PLATFORM_CXX_TEST(KWSYS_CXX_HAS_EXT_STDIO_FILEBUF_H
    "Checking whether <ext/stdio_filebuf.h> is available" DIRECT)
endif()

#-----------------------------------------------------------------------------
# Choose a directory for the generated headers.
if(NOT KWSYS_HEADER_ROOT)
  set(KWSYS_HEADER_ROOT "${PROJECT_BINARY_DIR}")
endif()
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
  endif()

  # Install the license under the documentation directory.
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt
    DESTINATION ${KWSYS_INSTALL_DOC_DIR}/${KWSYS_NAMESPACE}
    ${KWSYS_INSTALL_LICENSE_OPTIONS})
endif()

#-----------------------------------------------------------------------------
# Build a list of classes and headers we need to implement the
# selected components.  Initialize with required components.
set(KWSYS_CLASSES)
set(KWSYS_H_FILES Configure SharedForward)
set(KWSYS_HXX_FILES Configure String)

# Add selected C++ classes.
set(cppclasses
  Directory DynamicLoader Encoding Glob RegularExpression SystemTools
  CommandLineArguments FStream SystemInformation ConsoleBuf Status
  )
foreach(cpp ${cppclasses})
  if(KWSYS_USE_${cpp})
    # Use the corresponding class.
    set(KWSYS_CLASSES ${KWSYS_CLASSES} ${cpp})

    # Load component-specific CMake code.
    if(EXISTS ${PROJECT_SOURCE_DIR}/kwsys${cpp}.cmake)
      include(${PROJECT_SOURCE_DIR}/kwsys${cpp}.cmake)
    endif()
  endif()
endforeach()

# Add selected C components.
foreach(c
    Process Base64 Encoding MD5 Terminal System String
    )
  if(KWSYS_USE_${c})
    # Use the corresponding header file.
    set(KWSYS_H_FILES ${KWSYS_H_FILES} ${c})

    # Load component-specific CMake code.
    if(EXISTS ${PROJECT_SOURCE_DIR}/kwsys${c}.cmake)
      include(${PROJECT_SOURCE_DIR}/kwsys${c}.cmake)
    endif()
  endif()
endforeach()

#-----------------------------------------------------------------------------
# Build a list of sources for the library based on components that are
# included.
set(KWSYS_C_SRCS)
set(KWSYS_CXX_SRCS)

# Add the proper sources for this platform's Process implementation.
if(KWSYS_USE_Process)
  if(NOT UNIX)
    # Use the Windows implementation.
    set(KWSYS_C_SRCS ${KWSYS_C_SRCS} ProcessWin32.c)
  else()
    # Use the UNIX implementation.
    set(KWSYS_C_SRCS ${KWSYS_C_SRCS} ProcessUNIX.c)
  endif()
endif()

# Add selected C sources.
foreach(c Base64 Encoding MD5 Terminal System String)
  if(KWSYS_USE_${c})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${c}C.c)
      list(APPEND KWSYS_C_SRCS ${c}C.c)
    else()
      list(APPEND KWSYS_C_SRCS ${c}.c)
    endif()
  endif()
endforeach()

# Configure headers of C++ classes and construct the list of sources.
foreach(c ${KWSYS_CLASSES})
  # Add this source to the list of source files for the library.
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${c}CXX.cxx)
    list(APPEND KWSYS_CXX_SRCS ${c}CXX.cxx)
  elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${c}.cxx)
    list(APPEND KWSYS_CXX_SRCS ${c}.cxx)
  endif()

  # Configure the header for this class.
  configure_file(${PROJECT_SOURCE_DIR}/${c}.hxx.in ${KWSYS_HEADER_DIR}/${c}.hxx
                 @ONLY IMMEDIATE)
  set(KWSYS_CXX_SRCS ${KWSYS_CXX_SRCS} ${KWSYS_HEADER_DIR}/${c}.hxx)

  # Create an install target for the header.
  if(KWSYS_INSTALL_INCLUDE_DIR)
    install(FILES ${KWSYS_HEADER_DIR}/${c}.hxx
      DESTINATION ${KWSYS_INSTALL_INCLUDE_DIR}/${KWSYS_NAMESPACE}
      ${KWSYS_INSTALL_INCLUDE_OPTIONS})
  endif()
endforeach()

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
  endif()
endforeach()

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
  endif()
endforeach()

#-----------------------------------------------------------------------------
# Add the library with the configured name and list of sources.
if(KWSYS_C_SRCS OR KWSYS_CXX_SRCS)
  if(KWSYS_SPLIT_OBJECTS_FROM_INTERFACE)
    set(KWSYS_TARGET_INTERFACE ${KWSYS_NAMESPACE})
    set(KWSYS_TARGET_OBJECT ${KWSYS_NAMESPACE}_objects)
    set(KWSYS_TARGET_LINK ${KWSYS_NAMESPACE}_private)
    set(KWSYS_TARGET_INSTALL ${KWSYS_TARGET_INTERFACE} ${KWSYS_TARGET_LINK})
    set(KWSYS_LINK_DEPENDENCY INTERFACE)
    add_library(${KWSYS_TARGET_OBJECT} OBJECT
      ${KWSYS_C_SRCS} ${KWSYS_CXX_SRCS})
    if(KWSYS_BUILD_SHARED OR KWSYS_BUILD_PIC)
      set_property(TARGET ${KWSYS_TARGET_OBJECT} PROPERTY
        POSITION_INDEPENDENT_CODE TRUE)
    endif()
    add_library(${KWSYS_TARGET_INTERFACE} INTERFACE)
    add_library(${KWSYS_TARGET_LINK} INTERFACE)
    target_link_libraries(${KWSYS_TARGET_LINK} INTERFACE
      ${KWSYS_TARGET_INTERFACE})
    target_sources(${KWSYS_TARGET_LINK} INTERFACE
      $<TARGET_OBJECTS:${KWSYS_TARGET_OBJECT}>)
    target_compile_features(${KWSYS_TARGET_OBJECT} PRIVATE ${KWSYS_CXX_COMPILE_FEATURES})
    target_compile_features(${KWSYS_TARGET_INTERFACE} INTERFACE ${KWSYS_CXX_COMPILE_FEATURES})
  else()
    set(KWSYS_TARGET_INTERFACE ${KWSYS_NAMESPACE})
    set(KWSYS_TARGET_OBJECT ${KWSYS_NAMESPACE})
    set(KWSYS_TARGET_LINK ${KWSYS_NAMESPACE})
    set(KWSYS_TARGET_INSTALL ${KWSYS_TARGET_LINK})
    set(KWSYS_LINK_DEPENDENCY PUBLIC)
    add_library(${KWSYS_TARGET_INTERFACE} ${KWSYS_LIBRARY_TYPE}
      ${KWSYS_C_SRCS} ${KWSYS_CXX_SRCS})
    target_compile_features(${KWSYS_TARGET_INTERFACE} PUBLIC ${KWSYS_CXX_COMPILE_FEATURES})
  endif()
  if (KWSYS_ALIAS_TARGET)
    add_library(${KWSYS_ALIAS_TARGET} ALIAS ${KWSYS_TARGET_INTERFACE})
  endif ()
  set_target_properties(${KWSYS_TARGET_OBJECT} PROPERTIES
    C_CLANG_TIDY ""
    CXX_CLANG_TIDY ""
    C_INCLUDE_WHAT_YOU_USE ""
    CXX_INCLUDE_WHAT_YOU_USE ""
    LABELS "${KWSYS_LABELS_LIB}")
  if(KWSYS_USE_DynamicLoader)
    if(UNIX)
      target_link_libraries(${KWSYS_TARGET_INTERFACE} ${KWSYS_LINK_DEPENDENCY}
        ${CMAKE_DL_LIBS})
    endif()
  endif()

  if(KWSYS_USE_SystemInformation)
    if(WIN32)
      target_link_libraries(${KWSYS_TARGET_INTERFACE} ${KWSYS_LINK_DEPENDENCY} ws2_32)
      # link in dbghelp.dll for symbol lookup if MSVC 1800 or later
      # Note that the dbghelp runtime is part of MS Windows OS
      if(MSVC_VERSION AND NOT MSVC_VERSION VERSION_LESS 1800)
        target_link_libraries(${KWSYS_TARGET_INTERFACE} ${KWSYS_LINK_DEPENDENCY} dbghelp)
      endif()
      if(KWSYS_SYS_HAS_PSAPI)
        target_link_libraries(${KWSYS_TARGET_INTERFACE} ${KWSYS_LINK_DEPENDENCY}
          psapi)
      endif()
    elseif(UNIX)
      if (EXECINFO_LIB AND KWSYS_CXX_HAS_BACKTRACE)
        # backtrace on FreeBSD is not in libc
        target_link_libraries(${KWSYS_TARGET_INTERFACE} ${KWSYS_LINK_DEPENDENCY}
          ${EXECINFO_LIB})
      endif()
      if (KWSYS_CXX_HAS_DLADDR)
        # for symbol lookup using dladdr
        target_link_libraries(${KWSYS_TARGET_INTERFACE} ${KWSYS_LINK_DEPENDENCY}
          ${CMAKE_DL_LIBS})
      endif()
      if (CMAKE_SYSTEM_NAME STREQUAL "SunOS")
        target_link_libraries(${KWSYS_TARGET_INTERFACE} ${KWSYS_LINK_DEPENDENCY}
          socket)
      endif()
    endif()
  endif()

  # Apply user-defined target properties to the library.
  if(KWSYS_PROPERTIES_CXX)
    set_target_properties(${KWSYS_TARGET_INTERFACE} PROPERTIES
      ${KWSYS_PROPERTIES_CXX})
  endif()

  # Set up include usage requirement
  if(COMMAND TARGET_INCLUDE_DIRECTORIES)
    target_include_directories(${KWSYS_TARGET_INTERFACE} INTERFACE
      $<BUILD_INTERFACE:${KWSYS_HEADER_ROOT}>)
    if(KWSYS_INSTALL_INCLUDE_DIR)
      target_include_directories(${KWSYS_TARGET_INTERFACE} INTERFACE
        $<INSTALL_INTERFACE:${KWSYS_INSTALL_INCLUDE_DIR}>)
    endif()
  endif()

  # Create an install target for the library.
  if(KWSYS_INSTALL_LIBRARY_RULE)
    install(TARGETS ${KWSYS_TARGET_INSTALL} ${KWSYS_INSTALL_LIBRARY_RULE})
  endif()
  if(KWSYS_INSTALL_NAMELINK_RULE)
    install(TARGETS ${KWSYS_TARGET_INSTALL} ${KWSYS_INSTALL_NAMELINK_RULE})
  endif()
endif()

# Add a C-only library if requested.
if(KWSYS_ENABLE_C AND KWSYS_C_SRCS)
 if(KWSYS_SPLIT_OBJECTS_FROM_INTERFACE)
    set(KWSYS_TARGET_C_INTERFACE ${KWSYS_NAMESPACE}_c)
    set(KWSYS_TARGET_C_OBJECT ${KWSYS_NAMESPACE}_c_objects)
    set(KWSYS_TARGET_C_LINK ${KWSYS_NAMESPACE}_c_private)
    set(KWSYS_TARGET_C_INSTALL
      ${KWSYS_TARGET_C_INTERFACE} ${KWSYS_TARGET_C_LINK})
    set(KWSYS_LINK_DEPENDENCY INTERFACE)
    add_library(${KWSYS_TARGET_C_OBJECT} OBJECT ${KWSYS_C_SRCS})
    if(KWSYS_BUILD_SHARED OR KWSYS_BUILD_PIC)
      set_property(TARGET ${KWSYS_TARGET_C_OBJECT} PROPERTY
        POSITION_INDEPENDENT_CODE TRUE)
    endif()
    add_library(${KWSYS_TARGET_C_INTERFACE} INTERFACE)
    add_library(${KWSYS_TARGET_C_LINK} INTERFACE)
    target_link_libraries(${KWSYS_TARGET_C_LINK} INTERFACE
      ${KWSYS_TARGET_C_INTERFACE})
    target_sources(${KWSYS_TARGET_C_LINK} INTERFACE
      $<TARGET_OBJECTS:${KWSYS_TARGET_C_OBJECT}>)
  else()
    set(KWSYS_TARGET_C_INTERFACE ${KWSYS_NAMESPACE}_c)
    set(KWSYS_TARGET_C_OBJECT ${KWSYS_NAMESPACE}_c)
    set(KWSYS_TARGET_C_LINK ${KWSYS_NAMESPACE}_c)
    set(KWSYS_TARGET_C_INSTALL ${KWSYS_TARGET_C_LINK})
    set(KWSYS_LINK_DEPENDENCY PUBLIC)
    add_library(${KWSYS_TARGET_C_INTERFACE} ${KWSYS_LIBRARY_TYPE}
      ${KWSYS_C_SRCS})
  endif()
  set_target_properties(${KWSYS_TARGET_C_OBJECT} PROPERTIES
    LABELS "${KWSYS_LABELS_LIB}")

  # Apply user-defined target properties to the library.
  if(KWSYS_PROPERTIES_C)
    set_target_properties(${KWSYS_TARGET_C_INTERFACE} PROPERTIES
      ${KWSYS_PROPERTIES_C})
  endif()

  # Set up include usage requirement
  if(COMMAND TARGET_INCLUDE_DIRECTORIES)
    target_include_directories(${KWSYS_TARGET_C_INTERFACE} INTERFACE
      $<BUILD_INTERFACE:${KWSYS_HEADER_ROOT}>)
    if(KWSYS_INSTALL_INCLUDE_DIR)
      target_include_directories(${KWSYS_TARGET_C_INTERFACE} INTERFACE
        $<INSTALL_INTERFACE:${KWSYS_INSTALL_INCLUDE_DIR}>)
    endif()
  endif()

  # Create an install target for the library.
  if(KWSYS_INSTALL_LIBRARY_RULE)
    install(TARGETS ${KWSYS_TARGET_C_INSTALL})
  endif()
endif()

# For building kwsys itself, we use a macro defined on the command
# line to configure the namespace in the C and C++ source files.
add_definitions("-DKWSYS_NAMESPACE=${KWSYS_NAMESPACE}")

# Disable deprecation warnings for standard C functions.
if(MSVC OR (WIN32 AND (CMAKE_C_COMPILER_ID STREQUAL "Intel" OR
   (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))))
  add_definitions(
    -D_CRT_NONSTDC_NO_DEPRECATE
    -D_CRT_SECURE_NO_DEPRECATE
    -D_CRT_SECURE_NO_WARNINGS
    -D_SCL_SECURE_NO_DEPRECATE
    )
endif()

if(WIN32)
  # Help enforce the use of wide Windows apis.
  add_definitions(-DUNICODE -D_UNICODE)
endif()

if(KWSYS_USE_String)
  # Activate code in "String.c".  See the comment in the source.
  set_source_files_properties(String.c PROPERTIES
    COMPILE_FLAGS "-DKWSYS_STRING_C")
endif()

if(KWSYS_USE_Encoding)
  # Set default 8 bit encoding in "EndcodingC.c".
  set_property(SOURCE EncodingC.c EncodingCXX.cxx APPEND PROPERTY COMPILE_DEFINITIONS
    KWSYS_ENCODING_DEFAULT_CODEPAGE=${KWSYS_ENCODING_DEFAULT_CODEPAGE})
endif()

#-----------------------------------------------------------------------------
# Setup testing if not being built as part of another project.
if(KWSYS_STANDALONE OR CMake_SOURCE_DIR)
  if(BUILD_TESTING)
    # Compute the location of executables.
    set(EXEC_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
      set(EXEC_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endif()

    # C tests
    set(KWSYS_C_TESTS
      testEncode.c
      testTerminal.c
      )
    if(KWSYS_STANDALONE)
      set(KWSYS_C_TESTS ${KWSYS_C_TESTS} testFail.c)
    endif()
    create_test_sourcelist(
      KWSYS_C_TEST_SRCS ${KWSYS_NAMESPACE}TestsC.c
      ${KWSYS_C_TESTS}
      )
    add_executable(${KWSYS_NAMESPACE}TestsC ${KWSYS_C_TEST_SRCS})
    set_property(TARGET ${KWSYS_NAMESPACE}TestsC PROPERTY LABELS ${KWSYS_LABELS_EXE})
    target_link_libraries(${KWSYS_NAMESPACE}TestsC ${KWSYS_TARGET_C_LINK})
    foreach(testfile ${KWSYS_C_TESTS})
      get_filename_component(test "${testfile}" NAME_WE)
      add_test(kwsys.${test} ${EXEC_DIR}/${KWSYS_NAMESPACE}TestsC ${test} ${KWSYS_TEST_ARGS_${test}})
      set_property(TEST kwsys.${test} PROPERTY LABELS ${KWSYS_LABELS_TEST})
    endforeach()

    # C++ tests
    set(KWSYS_CXX_TESTS ${KWSYS_CXX_TESTS}
      testConfigure.cxx
      testStatus.cxx
      testSystemTools.cxx
      testCommandLineArguments.cxx
      testCommandLineArguments1.cxx
      testDirectory.cxx
      )
    if(KWSYS_STL_HAS_WSTRING)
      set(KWSYS_CXX_TESTS ${KWSYS_CXX_TESTS}
        testEncoding.cxx
        )
    endif()
    if(KWSYS_USE_FStream)
      set(KWSYS_CXX_TESTS ${KWSYS_CXX_TESTS}
        testFStream.cxx
        )
    endif()
    if(KWSYS_USE_ConsoleBuf)
      add_executable(testConsoleBufChild testConsoleBufChild.cxx)
      set_property(TARGET testConsoleBufChild PROPERTY C_CLANG_TIDY "")
      set_property(TARGET testConsoleBufChild PROPERTY CXX_CLANG_TIDY "")
      set_property(TARGET testConsoleBufChild PROPERTY C_INCLUDE_WHAT_YOU_USE "")
      set_property(TARGET testConsoleBufChild PROPERTY CXX_INCLUDE_WHAT_YOU_USE "")
      set_property(TARGET testConsoleBufChild PROPERTY LABELS ${KWSYS_LABELS_EXE})
      target_link_libraries(testConsoleBufChild ${KWSYS_TARGET_LINK})
      set(KWSYS_CXX_TESTS ${KWSYS_CXX_TESTS}
        testConsoleBuf.cxx
        )
      if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND
         CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "19.0.23506")
        set_property(SOURCE testConsoleBuf.cxx testConsoleBufChild.cxx PROPERTY COMPILE_FLAGS /utf-8)
      endif()
      set_property(SOURCE testConsoleBuf.cxx APPEND PROPERTY COMPILE_DEFINITIONS
        KWSYS_ENCODING_DEFAULT_CODEPAGE=${KWSYS_ENCODING_DEFAULT_CODEPAGE})
    endif()
    if(KWSYS_USE_SystemInformation)
      set(KWSYS_CXX_TESTS ${KWSYS_CXX_TESTS} testSystemInformation.cxx)
    endif()
    if(KWSYS_USE_DynamicLoader)
      set(KWSYS_CXX_TESTS ${KWSYS_CXX_TESTS} testDynamicLoader.cxx)
      # If kwsys contains the DynamicLoader, need extra library
      add_library(${KWSYS_NAMESPACE}TestDynload MODULE testDynload.c)
      set_property(TARGET ${KWSYS_NAMESPACE}TestDynload PROPERTY LABELS ${KWSYS_LABELS_LIB})
      add_dependencies(${KWSYS_NAMESPACE}TestDynload ${KWSYS_TARGET_INTERFACE})

      if (WIN32)
        # Windows tests supported flags.
        add_library(${KWSYS_NAMESPACE}TestDynloadImpl SHARED testDynloadImpl.c)
        set_property(TARGET ${KWSYS_NAMESPACE}TestDynloadImpl PROPERTY LABELS ${KWSYS_LABELS_LIB})
        set_property(TARGET ${KWSYS_NAMESPACE}TestDynloadImpl PROPERTY DEFINE_SYMBOL BUILDING_TestDynloadImpl)
        set_property(TARGET ${KWSYS_NAMESPACE}TestDynloadImpl PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/dynloaddir")
        add_dependencies(${KWSYS_NAMESPACE}TestDynloadImpl ${KWSYS_TARGET_INTERFACE})
        add_library(${KWSYS_NAMESPACE}TestDynloadUse MODULE testDynloadUse.c)
        set_property(TARGET ${KWSYS_NAMESPACE}TestDynloadUse PROPERTY LABELS ${KWSYS_LABELS_LIB})
        set_property(TARGET ${KWSYS_NAMESPACE}TestDynloadUse PROPERTY LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/dynloaddir")
        add_dependencies(${KWSYS_NAMESPACE}TestDynloadUse ${KWSYS_TARGET_INTERFACE})
        target_link_libraries(${KWSYS_NAMESPACE}TestDynloadUse PRIVATE ${KWSYS_NAMESPACE}TestDynloadImpl)
      endif ()
    endif()
    create_test_sourcelist(
      KWSYS_CXX_TEST_SRCS ${KWSYS_NAMESPACE}TestsCxx.cxx
      ${KWSYS_CXX_TESTS}
      )
    add_executable(${KWSYS_NAMESPACE}TestsCxx ${KWSYS_CXX_TEST_SRCS})
    set_property(TARGET ${KWSYS_NAMESPACE}TestsCxx PROPERTY C_CLANG_TIDY "")
    set_property(TARGET ${KWSYS_NAMESPACE}TestsCxx PROPERTY CXX_CLANG_TIDY "")
    set_property(TARGET ${KWSYS_NAMESPACE}TestsCxx PROPERTY C_INCLUDE_WHAT_YOU_USE "")
    set_property(TARGET ${KWSYS_NAMESPACE}TestsCxx PROPERTY CXX_INCLUDE_WHAT_YOU_USE "")
    set_property(TARGET ${KWSYS_NAMESPACE}TestsCxx PROPERTY LABELS ${KWSYS_LABELS_EXE})
    target_link_libraries(${KWSYS_NAMESPACE}TestsCxx ${KWSYS_TARGET_LINK})

    set(TEST_SYSTEMTOOLS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    set(TEST_SYSTEMTOOLS_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    configure_file(
      ${PROJECT_SOURCE_DIR}/testSystemTools.h.in
      ${PROJECT_BINARY_DIR}/testSystemTools.h)
    include_directories(${PROJECT_BINARY_DIR})

    if(CTEST_TEST_KWSYS)
      configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ExtraTest.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ExtraTest.cmake")
      set_directory_properties(PROPERTIES TEST_INCLUDE_FILE "${CMAKE_CURRENT_BINARY_DIR}/ExtraTest.cmake")
    endif()

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
    foreach(testfile ${KWSYS_CXX_TESTS})
      get_filename_component(test "${testfile}" NAME_WE)
      add_test(kwsys.${test} ${EXEC_DIR}/${KWSYS_NAMESPACE}TestsCxx ${test} ${KWSYS_TEST_ARGS_${test}})
      set_property(TEST kwsys.${test} PROPERTY LABELS ${KWSYS_LABELS_TEST})
    endforeach()

    # Process tests.
    add_executable(${KWSYS_NAMESPACE}TestProcess testProcess.c)
    set_property(TARGET ${KWSYS_NAMESPACE}TestProcess PROPERTY LABELS ${KWSYS_LABELS_EXE})
    target_link_libraries(${KWSYS_NAMESPACE}TestProcess ${KWSYS_TARGET_C_LINK})
    #set(KWSYS_TEST_PROCESS_7 7) # uncomment to run timing-sensitive test locally
    foreach(n 1 2 3 4 5 6 ${KWSYS_TEST_PROCESS_7} 9 10)
      add_test(kwsys.testProcess-${n} ${EXEC_DIR}/${KWSYS_NAMESPACE}TestProcess ${n})
      set_property(TEST kwsys.testProcess-${n} PROPERTY LABELS ${KWSYS_LABELS_TEST})
      set_tests_properties(kwsys.testProcess-${n} PROPERTIES TIMEOUT 120)
    endforeach()

    set(testProcess_COMPILE_FLAGS "")
    # Some Apple compilers produce bad optimizations in this source.
    if(APPLE AND CMAKE_C_COMPILER_ID MATCHES "^(GNU|LLVM)$")
      set(testProcess_COMPILE_FLAGS "${testProcess_COMPILE_FLAGS} -O0")
    elseif(CMAKE_C_COMPILER_ID MATCHES "^(XL|XLClang)$")
      # Tell IBM XL not to warn about our test infinite loop
      if(CMAKE_SYSTEM MATCHES "Linux.*ppc64le"
         AND CMAKE_C_COMPILER_VERSION VERSION_LESS "16.1.0"
         AND NOT CMAKE_C_COMPILER_VERSION VERSION_LESS "13.1.1")
        # v13.1.[1-6] on Linux ppc64le is clang based and does not accept
        # the -qsuppress option, so just suppress all warnings.
        set(testProcess_COMPILE_FLAGS "${testProcess_COMPILE_FLAGS} -w")
      else()
        set(testProcess_COMPILE_FLAGS "${testProcess_COMPILE_FLAGS} -qsuppress=1500-010")
      endif()
    endif()
    if(CMAKE_C_FLAGS MATCHES "-fsanitize=")
      set(testProcess_COMPILE_FLAGS "${testProcess_COMPILE_FLAGS} -DCRASH_USING_ABORT")
    endif()
    set_property(SOURCE testProcess.c PROPERTY COMPILE_FLAGS "${testProcess_COMPILE_FLAGS}")

    # Test SharedForward
    configure_file(${PROJECT_SOURCE_DIR}/testSharedForward.c.in
                   ${PROJECT_BINARY_DIR}/testSharedForward.c @ONLY IMMEDIATE)
    add_executable(${KWSYS_NAMESPACE}TestSharedForward
                   ${PROJECT_BINARY_DIR}/testSharedForward.c)
    set_property(TARGET ${KWSYS_NAMESPACE}TestSharedForward PROPERTY LABELS ${KWSYS_LABELS_EXE})
    add_dependencies(${KWSYS_NAMESPACE}TestSharedForward ${KWSYS_TARGET_C_LINK})
    add_test(kwsys.testSharedForward ${EXEC_DIR}/${KWSYS_NAMESPACE}TestSharedForward 1)
    set_property(TEST kwsys.testSharedForward PROPERTY LABELS ${KWSYS_LABELS_TEST})

    # Configure some test properties.
    if(KWSYS_STANDALONE)
      # We expect test to fail
      set_tests_properties(kwsys.testFail PROPERTIES WILL_FAIL ON)
      get_test_property(kwsys.testFail WILL_FAIL wfv)
      set_tests_properties(kwsys.testFail PROPERTIES MEASUREMENT "Some Key=Some Value")
      message(STATUS "GET_TEST_PROPERTY returned: ${wfv}")
    endif()

    # Set up ctest custom configuration file.
    configure_file(${PROJECT_SOURCE_DIR}/CTestCustom.cmake.in
                   ${PROJECT_BINARY_DIR}/CTestCustom.cmake @ONLY)

    # Suppress known consistent failures on buggy systems.
    if(KWSYS_TEST_BOGUS_FAILURES)
      set_tests_properties(${KWSYS_TEST_BOGUS_FAILURES} PROPERTIES WILL_FAIL ON)
    endif()

  endif()
endif()
