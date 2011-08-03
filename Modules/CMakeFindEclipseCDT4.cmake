
#=============================================================================
# Copyright 2009 Kitware, Inc.
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

# This file is included in CMakeSystemSpecificInformation.cmake if
# the Eclipse CDT4 extra generator has been selected.

FIND_PROGRAM(CMAKE_ECLIPSE_EXECUTABLE NAMES eclipse DOC "The Eclipse executable")

# This variable is used by the Eclipse generator and appended to the make invocation commands.
SET(CMAKE_ECLIPSE_MAKE_ARGUMENTS "" CACHE STRING "Additional command line arguments when Eclipse invokes make. Enter e.g. -j<some_number> to get parallel builds")

# This variable is used by the Eclipse generator in out-of-source builds only.
SET(ECLIPSE_CDT4_GENERATE_SOURCE_PROJECT FALSE CACHE BOOL "If enabled, CMake will generate a source project for Eclipse in CMAKE_SOURCE_DIR")
MARK_AS_ADVANCED(ECLIPSE_CDT4_GENERATE_SOURCE_PROJECT)

# Determine builtin macros and include dirs:
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/CMakeExtraGeneratorDetermineCompilerMacrosAndIncludeDirs.cmake)
