#
# This module finds if Java is installed and determines where the
# include files and libraries are. This code sets the following
# variables:
#
#  JAVA_RUNTIME        = the full path to the Java runtime
#  JAVA_COMPILE        = the full path to the Java compiler
#  JAVA_ARCHIVE        = the full path to the Java archiver
#

FIND_PROGRAM(JAVA_RUNTIME
  NAMES java
)

FIND_PROGRAM(JAVA_ARCHIVE
  NAMES jar
)

FIND_PROGRAM(JAVA_COMPILE
  NAMES javac
)

MARK_AS_ADVANCED(
JAVA_RUNTIME
JAVA_ARCHIVE
JAVA_COMPILE
)
