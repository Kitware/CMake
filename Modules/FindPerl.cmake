# 
# this module looks for Perl
#

INCLUDE(${CMAKE_ROOT}/Modules/FindCygwin.cmake)

IF (CYGWIN_INSTALL_PATH)

  FIND_PROGRAM(PERL
    perl
    "C:/Perl/bin" 
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActivePerl\\628]/bin
    ${CYGWIN_INSTALL_PATH}/bin
  )

ELSE (CYGWIN_INSTALL_PATH)

  FIND_PROGRAM(PERL
    perl
    "C:/Perl/bin" 
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActivePerl\\628]/bin
  )

ENDIF (CYGWIN_INSTALL_PATH)
