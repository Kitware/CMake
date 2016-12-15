#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
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
ENABLE_LANGUAGE(CXX)
file(WRITE
     ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/vc_system_include_paths.cxx
     "#ifndef __cplusplus\n"
     "# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"\n"
     "#endif\n"
     "int main () { }\n")

try_compile(VC_SYSTEM_INCLUDE_PATHS_RETURN
            ${CMAKE_BINARY_DIR}
            ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/vc_system_include_paths.cxx
            OUTPUT_VARIABLE
            VC_SYSTEM_INCLUDE_PATHS_OUTPUT)

unset(VC_SYSTEM_INCLUDE_PATHS_RETURN)
