# 
# this module looks for Perl
#

INCLUDE(${CMAKE_ROOT}/Modules/FindCygwin.cmake)

FIND_PROGRAM(PERL
  perl
  "C:/Perl/bin" 
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActivePerl\\628]/bin
  ${CYGWIN_INSTALL_PATH}/bin
)

MARK_AS_ADVANCED(
  PERL
)
