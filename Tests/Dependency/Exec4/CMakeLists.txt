# Even though Five's dependency on Two is explicitly satisfied, Two
# must be emitted again in order to satisfy a cyclic dependency on Three.
LINK_LIBRARIES( Five Two Five )

ADD_EXECUTABLE( exec4 ExecMain.c )

