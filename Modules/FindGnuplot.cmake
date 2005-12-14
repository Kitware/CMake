# - this module looks for gnuplot
#

INCLUDE(FindCygwin)

FIND_PROGRAM(GNUPLOT
  NAMES 
  gnuplot
  pgnuplot
  wgnupl32
  PATH
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin 
  /usr/local/bin
  /sbin
)
MARK_AS_ADVANCED(
  GNUPLOT
)
