# 
# this module looks for Cygwin
#

FIND_PATH(CYGWIN_INSTALL_PATH
  cygwin.bat
  "C:/Cygwin" 
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Cygnus Solutions\\Cygwin\\mounts v2\\/;native]"
)
