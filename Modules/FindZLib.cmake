#
# Find the native ZLIB includes and library
#
# ZLIB_INCLUDE_DIR - where to find zlib.h, etc.
# ZLIB_LIBRARIES   - List of fully qualified libraries to link against when using zlib.
# ZLIB_FOUND       - Do not attempt to use zlib if "no" or undefined.

FIND_PATH(ZLIB_INCLUDE_DIR zlib.h
  /usr/local/include
  /usr/include
)

FIND_LIBRARY(ZLIB_LIBRARY z
  /usr/lib
  /usr/local/lib
)

IF(ZLIB_INCLUDE_DIR)
  IF(ZLIB_LIBRARY)
    SET( ZLIB_LIBRARIES ${ZLIB_LIBRARY} )
    SET( ZLIB_FOUND "YES" )
  ENDIF(ZLIB_LIBRARY)
ENDIF(ZLIB_INCLUDE_DIR)
