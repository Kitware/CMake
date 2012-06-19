
#=============================================================================
# Copyright 2012 Kitware, Inc.
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

#
# When using Ninja cl.exe is wrapped by cmcldeps to extract the included
# headers for dependency tracking.
#
# cmcldeps path is set, and cmcldeps needs to know the localized string
# in front of each include path, so it can remove it.
#

IF(MSVC_C_ARCHITECTURE_ID AND CMAKE_GENERATOR MATCHES "Ninja" AND CMAKE_C_COMPILER AND CMAKE_COMMAND)
  STRING(REPLACE "cmake.exe" "cmcldeps.exe"  CMAKE_CMCLDEPS_EXECUTABLE ${CMAKE_COMMAND})
  SET(showdir ${CMAKE_BINARY_DIR}/CMakeFiles/ShowIncludes)
  FILE(WRITE ${showdir}/foo.h "\n")
  FILE(WRITE ${showdir}/main.c "#include \"foo.h\" \nint main(){}\n")
  EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} /nologo /showIncludes ${showdir}/main.c
                  WORKING_DIRECTORY ${showdir} OUTPUT_VARIABLE showOut)
  STRING(REPLACE main.c "" showOut1 ${showOut})
  STRING(REPLACE "/" "\\" header1 ${showdir}/foo.h)
  STRING(TOLOWER ${header1} header2)
  STRING(REPLACE ${header2} "" showOut2 ${showOut1})
  STRING(REPLACE "\n" "" showOut3 ${showOut2})
  SET(SET_CMAKE_CMCLDEPS_EXECUTABLE   "SET(CMAKE_CMCLDEPS_EXECUTABLE \"${CMAKE_CMCLDEPS_EXECUTABLE}\")")
  SET(SET_CMAKE_CL_SHOWINCLUDE_PREFIX "SET(CMAKE_CL_SHOWINCLUDE_PREFIX \"${showOut3}\")")
ENDIF()
