
#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
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

# This file is used by EnableLanguage in cmGlobalGenerator to
# determine that the selected ASM compiler works.
# For assembler this can only check whether the compiler has been found,
# because otherwise there would have to be a separate assembler source file
# for each assembler on every architecture.

IF(CMAKE_ASM${ASM_DIALECT}_COMPILER)
  SET(CMAKE_ASM${ASM_DIALECT}_COMPILER_WORKS 1 CACHE INTERNAL "")
ELSE(CMAKE_ASM${ASM_DIALECT}_COMPILER)
  SET(CMAKE_ASM${ASM_DIALECT}_COMPILER_WORKS 0 CACHE INTERNAL "")
ENDIF(CMAKE_ASM${ASM_DIALECT}_COMPILER)
