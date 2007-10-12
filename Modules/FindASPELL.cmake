# - Try to find ASPELL
# Once done this will define
#
#  ASPELL_FOUND - system has ASPELL
#  ASPELL_INCLUDE_DIR - the ASPELL include directory
#  ASPELL_LIBRARIES - The libraries needed to use ASPELL
#  ASPELL_DEFINITIONS - Compiler switches required for using ASPELL

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (ASPELL_INCLUDE_DIR AND ASPELL_LIBRARIES)
  # Already in cache, be silent
  SET(ASPELL_FIND_QUIETLY TRUE)
ENDIF (ASPELL_INCLUDE_DIR AND ASPELL_LIBRARIES)

FIND_PATH(ASPELL_INCLUDE_DIR aspell.h )

FIND_LIBRARY(ASPELL_LIBRARIES NAMES aspell aspell-15 libaspell-15 libaspell)

# handle the QUIETLY and REQUIRED arguments and set ASPELL_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ASPELL DEFAULT_MSG ASPELL_LIBRARIES ASPELL_INCLUDE_DIR)


MARK_AS_ADVANCED(ASPELL_INCLUDE_DIR ASPELL_LIBRARIES)
