# 
# this module looks for Doxygen and the path to Graphiz's dot
#

FIND_PROGRAM(DOXYGEN
  doxygen
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\doxygen_is1;Inno Setup: App Path]/bin"
)

FIND_PROGRAM(DOT
  dot
  "C:/Program Files/ATT/Graphviz/bin"
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\ATT\\Graphviz;InstallPath]/bin
)

MARK_AS_ADVANCED(
  DOT
  DOXYGEN
)
