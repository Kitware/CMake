PROJECT( LinkLine )

# Makes sure that the library order as specified by the user are
# unchanged by dependency analysis, etc.  libOne and libTwo are
# dependent on each other. The link line should be -lOne -lTwo -lOne.

ADD_LIBRARY( One One.c )
ADD_LIBRARY( Two Two.c )

LINK_LIBRARIES( One Two )
ADD_EXECUTABLE( Exec Exec.c )
LINK_LIBRARIES( One )
