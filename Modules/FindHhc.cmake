# 
# this module looks for Microsoft HTML Help Compiler
#

IF (WIN32)
  FIND_PROGRAM(HHC
    hhc
    "C:/Program Files/HTML Help Workshop" 
    "[HKEY_CURRENT_USER\\Software\\Microsoft\\HTML Help Workshop;InstallDir]"
  )

  MARK_AS_ADVANCED(
    HHC
  )
ENDIF (WIN32)
