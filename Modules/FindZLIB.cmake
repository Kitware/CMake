# - Find zlib
# Find the native ZLIB includes and library
#
#  ZLIB_INCLUDE_DIR - where to find zlib.h, etc.
#  ZLIB_LIBRARIES   - List of libraries when using zlib.
#  ZLIB_FOUND       - True if zlib found.

FIND_PATH(ZLIB_INCLUDE_DIR zlib.h
  /usr/local/include
  /usr/include
)

SET(ZLIB_NAMES ${ZLIB_NAMES} z zlib)
FIND_LIBRARY(ZLIB_LIBRARY
  NAMES ${ZLIB_NAMES}
  PATHS /usr/lib /usr/local/lib
)

IF(ZLIB_INCLUDE_DIR)
  IF(ZLIB_LIBRARY)
    SET( ZLIB_LIBRARIES ${ZLIB_LIBRARY} )
    SET( ZLIB_FOUND "YES" )
  ENDIF(ZLIB_LIBRARY)
ENDIF(ZLIB_INCLUDE_DIR)

MARK_AS_ADVANCED(
  ZLIB_LIBRARY
  ZLIB_INCLUDE_DIR
  )
