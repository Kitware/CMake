#
# try to find DCMTK libraries
#

# DCMTK_INCLUDE_DIR   - Directories to include to use DCMTK
# DCMTK_LIBRARIES     - Files to link against to use DCMTK
# DCMTK_FOUND         - If false, don't try to use DCMTK
# DCMTK_DIR           - (optional) Source directory for DCMTK
#
# DCMTK_DIR can be used to make it simpler to find the various include
# directories and compiled libraries if you've just compiled it in the
# source tree. Just set it to the root of the tree where you extracted
# the source.
#
# Written for VXL by Amitha Perera.
# 

FIND_PATH( DCMTK_config_INCLUDE_DIR osconfig.h
  ${DCMTK_DIR}/config/include
)

FIND_PATH( DCMTK_ofstd_INCLUDE_DIR ofstdinc.h
  ${DCMTK_DIR}/ofstd/include
)

FIND_LIBRARY( DCMTK_ofstd_LIBRARY ofstd
  ${DCMTK_DIR}/ofstd/libsrc
  ${DCMTK_DIR}/ofstd/Release
  ${DCMTK_DIR}/ofstd/Debug
)


FIND_PATH( DCMTK_dcmdata_INCLUDE_DIR dctypes.h
  ${DCMTK_DIR}/dcmdata/include
)

FIND_LIBRARY( DCMTK_dcmdata_LIBRARY dcmdata
  ${DCMTK_DIR}/dcmdata/libsrc
  ${DCMTK_DIR}/dcmdata/Release
  ${DCMTK_DIR}/dcmdata/Debug
)


FIND_PATH( DCMTK_dcmimgle_INCLUDE_DIR dcmimage.h
  ${DCMTK_DIR}/dcmimgle/include
)

FIND_LIBRARY( DCMTK_dcmimgle_LIBRARY dcmimgle
  ${DCMTK_DIR}/dcmimgle/libsrc
  ${DCMTK_DIR}/dcmimgle/Release
  ${DCMTK_DIR}/dcmimgle/Debug
)


IF( DCMTK_config_INCLUDE_DIR )
IF( DCMTK_ofstd_INCLUDE_DIR )
IF( DCMTK_ofstd_LIBRARY )
IF( DCMTK_dcmdata_INCLUDE_DIR )
IF( DCMTK_dcmdata_LIBRARY )
IF( DCMTK_dcmimgle_INCLUDE_DIR )
IF( DCMTK_dcmimgle_LIBRARY )

  SET( DCMTK_FOUND "YES" )
  SET( DCMTK_INCLUDE_DIR
    ${DCMTK_config_INCLUDE_DIR}
    ${DCMTK_ofstd_INCLUDE_DIR}
    ${DCMTK_dcmdata_INCLUDE_DIR}
    ${DCMTK_dcmimgle_INCLUDE_DIR}
  )

  SET( DCMTK_LIBRARIES
    ${DCMTK_dcmimgle_LIBRARY}
    ${DCMTK_dcmdata_LIBRARY}
    ${DCMTK_ofstd_LIBRARY}
    ${DCMTK_config_LIBRARY}
  )

  IF( WIN32 )
    SET( DCMTK_LIBRARIES ${DCMTK_LIBRARIES} netapi32 )
  ENDIF( WIN32 )

ENDIF( DCMTK_dcmimgle_LIBRARY )
ENDIF( DCMTK_dcmimgle_INCLUDE_DIR )
ENDIF( DCMTK_dcmdata_LIBRARY )
ENDIF( DCMTK_dcmdata_INCLUDE_DIR )
ENDIF( DCMTK_ofstd_LIBRARY )
ENDIF( DCMTK_ofstd_INCLUDE_DIR )
ENDIF( DCMTK_config_INCLUDE_DIR )

IF( NOT DCMTK_FOUND )
  SET( DCMTK_DIR "" CACHE PATH "Root of DCMTK source tree (optional)." )
  MARK_AS_ADVANCED( DCMTK_DIR )
ENDIF( NOT DCMTK_FOUND )
