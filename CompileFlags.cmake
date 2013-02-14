#=============================================================================
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

#-----------------------------------------------------------------------------
# set some special flags for different compilers
#
if(CMAKE_GENERATOR MATCHES "Visual Studio 7")
  set(CMAKE_SKIP_COMPATIBILITY_TESTS 1)
endif()
if(CMAKE_GENERATOR MATCHES "Visual Studio 6")
  set(CMAKE_SKIP_COMPATIBILITY_TESTS 1)
endif()
include (${CMAKE_ROOT}/Modules/CMakeBackwardCompatibilityCXX.cmake)

if(WIN32 AND "${CMAKE_C_COMPILER_ID}" MATCHES "^(Intel)$")
  set(_INTEL_WINDOWS 1)
endif()

# Disable deprecation warnings for standard C functions.
# really only needed for newer versions of VS, but should
# not hurt other versions, and this will work into the
# future
if(MSVC OR _INTEL_WINDOWS)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
else()
endif()

#silence duplicate symbol warnings on AIX
if(CMAKE_SYSTEM MATCHES "AIX.*")
  if(NOT CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -bhalt:5 ")
  endif()
endif()

if(CMAKE_SYSTEM MATCHES "IRIX.*")
  if(NOT CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-woff84 -no_auto_include")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-woff15")
  endif()
endif()

if(CMAKE_SYSTEM MATCHES "OSF1-V.*")
  if(NOT CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -timplicit_local -no_implicit_include ")
  endif()
endif()

# use the ansi CXX compile flag for building cmake
if (CMAKE_ANSI_CXXFLAGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_ANSI_CXXFLAGS}")
endif ()

if (CMAKE_ANSI_CFLAGS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")
endif ()

# avoid binutils problem with large binaries, e.g. when building CMake in debug mode
# See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=50230
if (CMAKE_SYSTEM_NAME STREQUAL Linux AND CMAKE_SYSTEM_PROCESSOR STREQUAL parisc)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--unique=.text.*")
endif ()
