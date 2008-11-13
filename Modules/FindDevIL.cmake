# This module locates the developer's image library.
# http://openil.sourceforge.net/
#
# This module sets:
# IL_LIBRARY the name of the IL library.
# ILU_LIBRARY the name of the ILU library.
# ILUT_LIBRARY the name of the ILUT library.
# IL_INCLUDE_DIR where to find the il.h, ilu.h and ilut.h files.
# IL_FOUND this is set to TRUE if all the above variables were set.

# Original file by: Christopher Harvey
# TODO: Add version support.
# Tested under Linux and Windows (MSVC)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PATH(IL_INCLUDE_DIR il.h 
  PATH_SUFFIXES include IL
  DOC "The path the the directory that contains il.h"
)

#MESSAGE("IL_INCLUDE_DIR is ${IL_INCLUDE_DIR}")

FIND_LIBRARY(IL_LIBRARY
  NAMES IL
  PATH_SUFFIXES lib64 lib lib32
  DOC "The file that corresponds to the base il library."
)

#MESSAGE("IL_LIBRARY is ${IL_LIBRARY}")

FIND_LIBRARY(ILUT_LIBRARY
  NAMES ILUT
  PATH_SUFFIXES lib64 lib lib32
  DOC "The file that corresponds to the il (system?) utility library."
)

#MESSAGE("ILUT_LIBRARY is ${ILUT_LIBRARY}")

FIND_LIBRARY(ILU_LIBRARY
  NAMES ILU
  PATH_SUFFIXES lib64 lib lib32
  DOC "The file that corresponds to the il utility library."
)

#MESSAGE("ILU_LIBRARY is ${ILU_LIBRARY}")

FIND_PACKAGE_HANDLE_STANDARD_ARGS(IL DEFAULT_MSG 
                                  IL_LIBRARY ILU_LIBRARY 
                                  ILUT_LIBRARY IL_INCLUDE_DIR)
