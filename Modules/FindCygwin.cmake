# - this module looks for Cygwin
#

IF (WIN32)
  FIND_PATH(CYGWIN_INSTALL_PATH
    cygwin.bat
    "C:/Cygwin" 
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Cygnus Solutions\\Cygwin\\mounts v2\\/;native]"
  )

  MARK_AS_ADVANCED(
    CYGWIN_INSTALL_PATH
  )
ENDIF (WIN32)
