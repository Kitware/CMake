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
#                                |           
#       ^           ^  ^         | ^         ^  ^
#       |           |  +-----+   | |         |  +----+
#       |           |        |   | |         |       |
#       +--------- Three  <------+ +------- SixA    SixB
#                            |                       |
#                            +-----------------------+
# NoDepA:
# NoDepB: NoDepA
# NoDepC: NoDepA
# One:
# Two: Three
# Three: One Four
# Four: One Two A
# Five: Two
# SixA: Two Five
# SixB: Four Five
# Exec: NoDepB NoDepC SixA SixB
#
# The libraries One,...,Five have their dependencies explicitly
# encoded. The libraries NoDepA,...,NoDepC do not.
#
# Although SixB does not depend on Two, there is a dependency listed
# in the corresponding CMakeLists.txt just because of commands used.

SUBDIRS( NoDepA NoDepB NoDepC )
SUBDIRS( One Two Three Four Five Six )
SUBDIRS( Exec )
