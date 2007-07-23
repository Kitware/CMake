# - Try to find Motif (or lesstif)
# Once done this will define:
#  MOTIF_FOUND        - system has MOTIF
#  MOTIF_INCLUDE_DIR  - incude paths to use Motif
#  MOTIF_LIBRARIES    - Link these to use Motif

SET(MOTIF_FOUND 0)

IF(UNIX)
  FIND_PATH(MOTIF_INCLUDE_DIR
    Xm/Xm.h
    /usr/openwin/include
    )

  FIND_LIBRARY(MOTIF_LIBRARIES
    Xm
    /usr/openwin/lib
    )

ENDIF(UNIX)

# handle the QUIETLY and REQUIRED arguments and set MOTIF_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Motif DEFAULT_MSG MOTIF_LIBRARIES MOTIF_INCLUDE_DIR)


MARK_AS_ADVANCED(
  MOTIF_INCLUDE_DIR
  MOTIF_LIBRARIES
)
