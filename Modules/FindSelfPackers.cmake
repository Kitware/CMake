# - Find upx
# This module looks for some executable packers (i.e. softwares that
# compress executables or shared libs into on-the-fly self-extracting
# executables or shared libs.
# Examples:
#  UPX: http://wildsau.idv.uni-linz.ac.at/mfx/upx.html

INCLUDE(FindCygwin)

FIND_PROGRAM(SELF_PACKER_FOR_EXECUTABLE
  upx
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin 
  /usr/local/bin
  /sbin
)

FIND_PROGRAM(SELF_PACKER_FOR_SHARED_LIB
  upx
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin 
  /usr/local/bin
  /sbin
)

MARK_AS_ADVANCED(
  SELF_PACKER_FOR_EXECUTABLE
  SELF_PACKER_FOR_SHARED_LIB
)

#
# Set flags
#
IF (SELF_PACKER_FOR_EXECUTABLE MATCHES "upx")
  SET (SELF_PACKER_FOR_EXECUTABLE_FLAGS "-q" CACHE STRING 
       "Flags for the executable self-packer.")
ELSE (SELF_PACKER_FOR_EXECUTABLE MATCHES "upx")
  SET (SELF_PACKER_FOR_EXECUTABLE_FLAGS "" CACHE STRING 
       "Flags for the executable self-packer.")
ENDIF (SELF_PACKER_FOR_EXECUTABLE MATCHES "upx")

IF (SELF_PACKER_FOR_SHARED_LIB MATCHES "upx")
  SET (SELF_PACKER_FOR_SHARED_LIB_FLAGS "-q" CACHE STRING 
       "Flags for the shared lib self-packer.")
ELSE (SELF_PACKER_FOR_SHARED_LIB MATCHES "upx")
  SET (SELF_PACKER_FOR_SHARED_LIB_FLAGS "" CACHE STRING 
       "Flags for the shared lib self-packer.")
ENDIF (SELF_PACKER_FOR_SHARED_LIB MATCHES "upx")

MARK_AS_ADVANCED(
  SELF_PACKER_FOR_EXECUTABLE_FLAGS
  SELF_PACKER_FOR_SHARED_LIB_FLAGS
)
