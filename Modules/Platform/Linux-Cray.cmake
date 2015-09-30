
#=============================================================================
# Copyright 2015 Kitware, Inc.
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

# This module is shared by multiple languages; use include blocker.
if(__LINUX_COMPILER_CRAY)
  return()
endif()
set(__LINUX_COMPILER_CRAY 1)

macro(__linux_compiler_cray lang)
endmacro()
