# - Try to find HSPELL
# Once done this will define
#
#  HSPELL_FOUND - system has HSPELL
#  HSPELL_INCLUDE_DIR - the HSPELL include directory
#  HSPELL_LIBRARIES - The libraries needed to use HSPELL
#  HSPELL_DEFINITIONS - Compiler switches required for using HSPELL

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (HSPELL_INCLUDE_DIR AND HSPELL_LIBRARIES)
  # Already in cache, be silent
  SET(HSPELL_FIND_QUIETLY TRUE)
ENDIF (HSPELL_INCLUDE_DIR AND HSPELL_LIBRARIES)


FIND_PATH(HSPELL_INCLUDE_DIR hspell.h )

FIND_LIBRARY(HSPELL_LIBRARIES NAMES hspell )

# handle the QUIETLY and REQUIRED arguments and set HSPELL_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HSPELL DEFAULT_MSG HSPELL_LIBRARIES HSPELL_INCLUDE_DIR)


MARK_AS_ADVANCED(HSPELL_INCLUDE_DIR HSPELL_LIBRARIES)

