# - Find TIFF library
# Find the native TIFF includes and library
# This module defines
#  TIFF_INCLUDE_DIR, where to find tiff.h, etc.
#  TIFF_LIBRARIES, libraries to link against to use TIFF.
#  TIFF_FOUND, If false, do not try to use TIFF.
# also defined, but not for general use are
#  TIFF_LIBRARY, where to find the TIFF library.

FIND_PATH(TIFF_INCLUDE_DIR tiff.h
  /usr/local/include
  /usr/include
)

IF(BORLAND)
  SET(TIFF_NAMES ${TIFF_NAMES} libtiff-bcc)
ENDIF(BORLAND)
SET(TIFF_NAMES ${TIFF_NAMES} tiff libtiff)

FIND_LIBRARY(TIFF_LIBRARY
  NAMES ${TIFF_NAMES}
  PATHS /usr/lib /usr/local/lib
  )

IF(TIFF_INCLUDE_DIR)
  IF(TIFF_LIBRARY)
    SET( TIFF_FOUND "YES" )
    SET( TIFF_LIBRARIES ${TIFF_LIBRARY} )
  ENDIF(TIFF_LIBRARY)
ENDIF(TIFF_INCLUDE_DIR)

