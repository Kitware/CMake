# - this module looks for gnuplot
#
# Once done this will define
#
#  GNUPLOT_FOUND - system has Gnuplot
#  GNUPLOT_EXECUTABLE - the Gnuplot executable

INCLUDE(FindCygwin)

FIND_PROGRAM(GNUPLOT_EXECUTABLE
  NAMES 
  gnuplot
  pgnuplot
  wgnupl32
  PATHS
  ${CYGWIN_INSTALL_PATH}/bin
)

# for compatibility
SET(GNUPLOT ${GNUPLOT_EXECUTABLE})

# handle the QUIETLY and REQUIRED arguments and set GNUPLOT_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Gnuplot DEFAULT_MSG GNUPLOT_EXECUTABLE)

MARK_AS_ADVANCED( GNUPLOT_EXECUTABLE )

