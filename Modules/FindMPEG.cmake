# - Find the native MPEG includes and library
# This module defines
#  MPEG_INCLUDE_DIR, where to find MPEG.h, etc.
#  MPEG_LIBRARIES, the libraries required to use MPEG.
#  MPEG_FOUND, If false, do not try to use MPEG.
# also defined, but not for general use are
#  MPEG_mpeg2_LIBRARY, where to find the MPEG library.
#  MPEG_vo_LIBRARY, where to find the vo library.

FIND_PATH(MPEG_INCLUDE_DIR mpeg2dec/include/video_out.h
  /usr/local/include
  /usr/include
  /usr/local/livid
)

FIND_LIBRARY(MPEG_mpeg2_LIBRARY mpeg2
  /usr/local/lib
  /usr/lib
  /usr/local/livid/mpeg2dec/libmpeg2/.libs
)

FIND_LIBRARY( MPEG_vo_LIBRARY vo
  /usr/local/lib
  /usr/lib
  /usr/local/livid/mpeg2dec/libvo/.libs
)

SET( MPEG_FOUND "NO" )
IF(MPEG_INCLUDE_DIR)
  IF(MPEG_mpeg2_LIBRARY)
    SET( MPEG_FOUND "YES" )
    SET( MPEG_LIBRARIES ${MPEG_mpeg2_LIBRARY} ${MPEG_vo_LIBRARY} )
  ENDIF(MPEG_mpeg2_LIBRARY)
ENDIF(MPEG_INCLUDE_DIR)

