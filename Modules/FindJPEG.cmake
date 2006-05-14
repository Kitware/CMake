# - Find JPEG
# Find the native JPEG includes and library
# This module defines
#  JPEG_INCLUDE_DIR, where to find jpeglib.h, etc.
#  JPEG_LIBRARIES, the libraries needed to use JPEG.
#  JPEG_FOUND, If false, do not try to use JPEG.
# also defined, but not for general use are
#  JPEG_LIBRARY, where to find the JPEG library.

FIND_PATH(JPEG_INCLUDE_DIR jpeglib.h
/usr/local/include
/usr/include
)

SET(JPEG_NAMES ${JPEG_NAMES} jpeg)
FIND_LIBRARY(JPEG_LIBRARY
  NAMES ${JPEG_NAMES}
  PATHS /usr/lib /usr/local/lib
  )

IF (JPEG_LIBRARY AND JPEG_INCLUDE_DIR)
    SET(JPEG_LIBRARIES ${JPEG_LIBRARY})
    SET(JPEG_FOUND "YES")
ELSE (JPEG_LIBRARY AND JPEG_INCLUDE_DIR)
  SET(JPEG_FOUND "NO")
ENDIF (JPEG_LIBRARY AND JPEG_INCLUDE_DIR)


IF (JPEG_FOUND)
   IF (NOT JPEG_FIND_QUIETLY)
      MESSAGE(STATUS "Found JPEG: ${JPEG_LIBRARIES}")
   ENDIF (NOT JPEG_FIND_QUIETLY)
ELSE (JPEG_FOUND)
   IF (JPEG_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find JPEG library")
   ENDIF (JPEG_FIND_REQUIRED)
ENDIF (JPEG_FOUND)

# Deprecated declarations.
SET (NATIVE_JPEG_INCLUDE_PATH ${JPEG_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_JPEG_LIB_PATH ${JPEG_LIBRARY} PATH)

MARK_AS_ADVANCED(
  JPEG_LIBRARY
  JPEG_INCLUDE_DIR
  )
