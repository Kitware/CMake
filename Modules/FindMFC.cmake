#
# Find the native MFC - i.e. decide if this is an MS VC box.
#
# MFC_FOUND       - Do not attempt to use MFC if "no" or undefined.
# You don't need to include anything or link anything to use it.

# Assume no MFC support
SET( MFC_FOUND "NO" )
# Add MFC support if win32 and not cygwin and not borland
IF( WIN32 )
  IF( NOT CYGWIN )
     IF( NOT BORLAND )
       SET( MFC_FOUND "YES" )
     ENDIF( NOT BORLAND )
  ENDIF( NOT CYGWIN )
ENDIF( WIN32 )


