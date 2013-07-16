#=============================================================================
# Copyright 2010 Kitware, Inc.
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

set(CMAKE_Fortran_VERBOSE_FLAG "-Wl,-v") # Runs gcc under the hood.

# Need -fpp explicitly on case-insensitive filesystem.
set(CMAKE_Fortran_COMPILE_OBJECT
  "<CMAKE_Fortran_COMPILER> -fpp -o <OBJECT> <DEFINES> <FLAGS> -c <SOURCE>")
