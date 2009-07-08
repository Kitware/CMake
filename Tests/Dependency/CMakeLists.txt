cmake_minimum_required (VERSION 2.6)
PROJECT( Dependency )

# to test directories with only one character One was changed to 1
# There is one executable that depends on eight libraries. The
# system has the following dependency graph:
#
# NoDepA:
# NoDepB: NoDepA
# NoDepC: NoDepA
# 1:
# Two: Three
# Three: 1 Four
# Four: 1 Two NoDepA
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

ADD_SUBDIRECTORY(NoDepA)
ADD_SUBDIRECTORY(NoDepB)
ADD_SUBDIRECTORY(NoDepC)
ADD_SUBDIRECTORY(1)
ADD_SUBDIRECTORY(Two)
ADD_SUBDIRECTORY(Three)
ADD_SUBDIRECTORY(Four)
ADD_SUBDIRECTORY(Five)
ADD_SUBDIRECTORY(Six)
ADD_SUBDIRECTORY(Seven)
ADD_SUBDIRECTORY(Eight)
ADD_SUBDIRECTORY(Exec)
ADD_SUBDIRECTORY(Exec2)
ADD_SUBDIRECTORY(Exec3)
ADD_SUBDIRECTORY(Exec4)

# Specific cases added to test fixes to problems found in real
# projects.
ADD_SUBDIRECTORY(Case1)
ADD_SUBDIRECTORY(Case2)
ADD_SUBDIRECTORY(Case3)
ADD_SUBDIRECTORY(Case4)
