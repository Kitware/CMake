# 
# this module looks for Doxygen and the path to Graphiz's dot
#

FIND_PROGRAM(DOXYGEN
  doxygen
)

FIND_PROGRAM(DOT
  dot
  "C:/Program Files/ATT/Graphviz/bin"
)
# HKEY_CURRENT_USER\Software\AT&T\Graphviz

# Since most of the time dot is called by Doxygen, the path to dot is useful too
GET_FILENAME_COMPONENT(DOT_PATH ${DOT} PATH CACHE)


