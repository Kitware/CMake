# - Find MFC on Windows
# Find the native MFC - i.e. decide if this is an MS VC box.
#  MFC_FOUND - Was MFC support found
# You don't need to include anything or link anything to use it.

# Assume no MFC support
SET( MFC_FOUND "NO" )
# Add MFC support if win32 and not cygwin and not borland
IF( WIN32 )
  IF( NOT CYGWIN )
    IF( NOT BORLAND )
      IF( NOT MINGW )
        SET( MFC_FOUND "YES" )
      ENDIF( NOT MINGW )
    ENDIF( NOT BORLAND )
  ENDIF( NOT CYGWIN )
ENDIF( WIN32 )


