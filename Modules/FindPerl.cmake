# 
# this module looks for Perl
#
# PERL_EXECUTABLE - the full path to the Perl interpreter
# PERL_FOUND	  - If false, don't attempt to use perl.

INCLUDE(${CMAKE_ROOT}/Modules/FindCygwin.cmake)

FIND_PROGRAM(PERL_EXECUTABLE
  perl
  "C:/Perl/bin" 
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

