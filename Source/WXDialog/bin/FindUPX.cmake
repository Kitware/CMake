#
# Find the UPX packer program
#
# UPX_PROGRAM_PATH      The full path to the UPX command (or UPX_PROGRAM_PATH-NOTFOUND when not found)
# UPX_FOUND             Is set to 1 when upx is found

FIND_PATH(UPX_PROGRAM_PATH upx.exe
  ${UPX_DIR}
  $ENV{UPX_DIR}
  "$ENV{ProgramFiles}/upx"
  )

# when found, note this as target
IF(UPX_PROGRAM_PATH)
  SET(UPX_FOUND 1)
ELSE(UPX_PROGRAM_PATH)
  SET(UPX_FOUND 0)
ENDIF(UPX_PROGRAM_PATH)
