ADD_EXECUTABLE( exec ExecMain.c )

# This executable directly depends on NoDepB, NoDepC and Five. However,
# since NoDepB and NoDepC do not have explicit dependency information,
# and they depend on NoDepA, we have to manually specify that dependency.
LINK_LIBRARIES( NoDepB NoDepC NoDepA Five )
