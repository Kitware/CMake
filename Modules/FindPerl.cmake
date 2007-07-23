# - Find perl
# this module looks for Perl
#
#  PERL_EXECUTABLE - the full path to perl
#  PERL_FOUND      - If false, don't attempt to use perl.

INCLUDE(FindCygwin)

FIND_PROGRAM(PERL_EXECUTABLE
  perl
  "C:/Perl/bin" 
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActivePerl\\804]/bin
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActivePerl\\628]/bin
  ${CYGWIN_INSTALL_PATH}/bin
  )


# Deprecated settings for compatibility with CMake1.4
SET (PERL ${PERL_EXECUTABLE})

# handle the QUIETLY and REQUIRED arguments and set PERL_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Perl DEFAULT_MSG PERL_EXECUTABLE)

MARK_AS_ADVANCED( PERL_EXECUTABLE )
