#
# This module finds if Java is installed and determines where the
# include files and libraries are. This code sets the following
# variables:
#
#  JAVA_RUNTIME        = the full path to the Java runtime
#  JAVA_COMPILE        = the full path to the Java compiler
#  JAVA_ARCHIVE        = the full path to the Java archiver
#

SET(JAVA_BIN_PATH
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.4;JavaHome]/bin"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.3;JavaHome]/bin"
  /usr/bin
  /usr/lib/java/bin
  /usr/share/java/bin
  /usr/local/bin
  /usr/local/java/bin
  /usr/java/j2sdk1.4.2_04
  )
FIND_PROGRAM(JAVA_RUNTIME
  NAMES java
  PATHS ${JAVA_BIN_PATH}
)

FIND_PROGRAM(JAVA_ARCHIVE
  NAMES jar
  PATHS ${JAVA_BIN_PATH}
)

FIND_PROGRAM(JAVA_COMPILE
  NAMES javac
  PATHS ${JAVA_BIN_PATH}
)

MARK_AS_ADVANCED(
JAVA_RUNTIME
JAVA_ARCHIVE
JAVA_COMPILE
)
