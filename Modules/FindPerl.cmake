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

MARK_AS_ADVANCED(
  PERL_EXECUTABLE
  )

IF (NOT PERL_EXECUTABLE)
  SET(PERL_FOUND "NO")
ELSE (NOT PERL_EXECUTABLE)
  SET(PERL_FOUND "YES")

  # Deprecated settings for compatibility with CMake1.4
  SET (PERL ${PERL_EXECUTABLE})
ENDIF (NOT PERL_EXECUTABLE)


IF (NOT PERL_FOUND AND Perl_FIND_REQUIRED)
   MESSAGE(FATAL_ERROR "Could not find Perl")
ENDIF (NOT PERL_FOUND AND Perl_FIND_REQUIRED)
