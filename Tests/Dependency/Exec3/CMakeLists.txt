# Here, Five already has it's immediate dependency, Two satisfied. We must
# make sure Two gets output anyway, because Eight indirectly depends on it.
LINK_LIBRARIES( Five Two Eight Five )

ADD_EXECUTABLE( exec3 ExecMain.c )

