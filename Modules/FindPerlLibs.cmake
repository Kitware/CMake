# - Find Perl libraries
# This module finds if PERL is installed and determines where the include files
# and libraries are. It also determines what the name of the library is. This
# code sets the following variables:
#
#  PERL_INCLUDE_PATH = path to where perl.h is found
#  PERL_EXECUTABLE   = full path to the perl binary
#

SET(PERL_POSSIBLE_INCLUDE_PATHS
  /usr/lib/perl/5.8.3/CORE
  /usr/lib/perl/5.8.2/CORE
  /usr/lib/perl/5.8.1/CORE
  /usr/lib/perl/5.8.0/CORE
  /usr/lib/perl/5.8/CORE
  )

SET(PERL_POSSIBLE_LIB_PATHS
  /usr/lib
  )

FIND_PATH(PERL_INCLUDE_PATH perl.h
  ${PERL_POSSIBLE_INCLUDE_PATHS})

# find the perl executable
INCLUDE(FindPerl)


IF(PERL_EXECUTABLE)
  EXEC_PROGRAM(${PERL_EXECUTABLE}
    ARGS -e "'use Config; print \$Config{libperl}, \"\\n\"'"
    OUTPUT_VARIABLE PERL_LIBRARY_OUTPUT_VARIABLE
    RETURN_VALUE PERL_LIBRARY_RETURN_VALUE
    )
  IF(NOT PERL_LIBRARY_RETURN_VALUE)
    FOREACH(path ${PERL_POSSIBLE_LIB_PATHS})
      SET(PERL_POSSIBLE_LIBRARY_NAME ${PERL_POSSIBLE_LIBRARY_NAME} "${path}/${PERL_LIBRARY_OUTPUT_VARIABLE}")
    ENDFOREACH(path ${PERL_POSSIBLE_LIB_PATHS})
  ENDIF(NOT PERL_LIBRARY_RETURN_VALUE)
  EXEC_PROGRAM(${PERL_EXECUTABLE}
    ARGS -e "'use Config; print \$Config{cppflags}, \"\\n\"'"
    OUTPUT_VARIABLE PERL_CPPFLAGS_OUTPUT_VARIABLE
    RETURN_VALUE PERL_CPPFLAGS_RETURN_VALUE
    )
  IF(NOT PERL_CPPFLAGS_RETURN_VALUE)
    SET(PERL_EXTRA_C_FLAGS ${PERL_CPPFLAGS_OUTPUT_VARIABLE})
  ENDIF(NOT PERL_CPPFLAGS_RETURN_VALUE)
ENDIF(PERL_EXECUTABLE)

FIND_LIBRARY(PERL_LIBRARY
  NAMES ${PERL_POSSIBLE_LIBRARY_NAME} perl5.8.0
  PATHS ${PERL_POSSIBLE_LIB_PATHS}
  )

# handle the QUIETLY and REQUIRED arguments and set PERLLIBS_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PerlLibs DEFAULT_MSG PERL_LIBRARY PERL_INCLUDE_PATH)


MARK_AS_ADVANCED(
  PERL_INCLUDE_PATH
  PERL_EXECUTABLE
  PERL_LIBRARY
  )
