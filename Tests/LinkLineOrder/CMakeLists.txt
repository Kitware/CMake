PROJECT( LinkLineOrder )

# This tests ensures that the order of libraries are preserved when
# they don't have dependency information, even if they are deep in the
# dependency tree.

# NoDepC depends on NoDepA which depends on NoDepB. NoDepE and NoDepF
# are dependent on each other (recursive dependency). However, CMake
# has no information about these libraries except for the order they
# are specified in One. We must make sure we don't lose that.

ADD_LIBRARY( NoDepA NoDepA.c )
ADD_LIBRARY( NoDepB NoDepB.c )
ADD_LIBRARY( NoDepC NoDepC.c )
ADD_LIBRARY( NoDepE NoDepE.c )
ADD_LIBRARY( NoDepF NoDepF.c )

ADD_LIBRARY( One One.c )
TARGET_LINK_LIBRARIES( One NoDepC NoDepA NoDepB NoDepE NoDepF NoDepE )

ADD_EXECUTABLE( Exec1 Exec1.c )
TARGET_LINK_LIBRARIES( Exec1 One )


# Similar situation as One, except at a different level of the
# dependency tree. This makes sure that the order is presevered
# everywhere in the graph.
ADD_LIBRARY( NoDepX NoDepX.c )
ADD_LIBRARY( NoDepY NoDepY.c )
ADD_LIBRARY( NoDepZ NoDepZ.c )

ADD_LIBRARY( Two Two.c )
TARGET_LINK_LIBRARIES( Two One NoDepZ NoDepX NoDepY )

ADD_EXECUTABLE( Exec2 Exec2.c )
TARGET_LINK_LIBRARIES( Exec2 Two )
