PROJECT( Dependency )

# There is one executable that depends on eight libraries. The
# system has the following dependency graph:
#
# NoDepA:
# NoDepB: NoDepA
# NoDepC: NoDepA
# One:
# Two: Three
# Three: One Four
# Four: One Two NoDepA
# Five: Two
# SixA: Two Five
# SixB: Four Five
# Seven: Two
# Eight: Seven
#
# Exec: NoDepB NoDepC SixA SixB
# Exec2: Eight Five
# Exec3: Eight Five
# Exec4: Five Two
#
# The libraries One,...,Eight have their dependencies explicitly
# encoded. The libraries NoDepA,...,NoDepC do not.
#
# Although SixB does not depend on Two, there is a dependency listed
# in the corresponding CMakeLists.txt just because of commands used.

SUBDIRS( NoDepA NoDepB NoDepC )
SUBDIRS( One Two Three Four Five Six Seven Eight )
SUBDIRS( Exec Exec2 Exec3 Exec4 )
