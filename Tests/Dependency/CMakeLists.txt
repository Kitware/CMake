PROJECT( Dependency )

SET( LIBRARY_OUTPUT_PATH ${Dependency_BINARY_DIR}/Lib )
SET( CMAKE_ANALYZE_LIB_DEPENDS "ON" )

# There is one executable that depends on eight libraries. The
# system has the following dependency graph:
#
#                    +----------- NoDepC  <----  EXECUTABLE
#                    |                            |    |
#                    V                            |    |
#                                                 |    |
#                  NoDepA  <----- NoDepB  <-------+    |
#                                                      |
#                    ^                                 |
#                    |                                 |
#      One  <------ Four ----->  Two  <----- Five  <---+
#                                 |
#       ^            ^            |
#       |            |            |
#       +--------- Three  <-------+
#
# NoDepA:
# NoDepB: NoDepA
# NoDepC: NoDepA
# One:
# Two: Three
# Three: One Four
# Four: One Two A
# Five: Two
# Exec: NoDepB NoDepC Five
#
# The libraries One,...,Five have their dependencies explicitly
# encoded. The libraries NoDepA,...,NoDepC do not.

SUBDIRS( NoDepA NoDepB NoDepC )
SUBDIRS( One Two Three Four Five )
SUBDIRS( Exec )
