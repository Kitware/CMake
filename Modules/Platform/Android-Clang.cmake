#=============================================================================
# Copyright 2015-2016 Kitware, Inc.
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
if(__ANDROID_COMPILER_CLANG)
  return()
endif()
set(__ANDROID_COMPILER_CLANG 1)

include(Platform/Android-Common)

macro(__android_compiler_clang lang)
  __android_compiler_common(${lang})
endmacro()
