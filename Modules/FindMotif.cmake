# - Try to find Motif (or lesstif)
# Once done this will define:
#  MOTIF_FOUND        - system has MOTIF
#  MOTIF_INCLUDE_DIR  - incude paths to use Motif
#  MOTIF_LIBRARIES    - Link these to use Motif

SET(MOTIF_FOUND 0)
IF(UNIX)
  FIND_PATH(MOTIF_INCLUDE_DIR
    Xm/Xm.h
    /usr/X11R6/include
    /usr/local/include
    /usr/openwin/include
    /usr/include
    )

  FIND_LIBRARY(MOTIF_LIBRARIES
    Xm
    /usr/X11R6/lib
    /usr/local/lib
    /usr/openwin/lib
    /usr/lib
    )

  IF(MOTIF_LIBRARIES AND MOTIF_INCLUDE_DIR)
    SET(MOTIF_FOUND 1)
  ENDIF(MOTIF_LIBRARIES AND MOTIF_INCLUDE_DIR)
ENDIF(UNIX)

MARK_AS_ADVANCED(
  MOTIF_INCLUDE_DIR
  MOTIF_LIBRARIES
)
