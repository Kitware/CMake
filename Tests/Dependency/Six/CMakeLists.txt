# In some projects, people don't use TARGET_LINK_LIBRARIES, but just
# use an all-encompassing LINK_LIBRARIES. And sometimes they don't
# specify them in the correct order.

LINK_LIBRARIES( Two )
LINK_LIBRARIES( Five )

ADD_LIBRARY( SixA SixASrc.c )

ADD_LIBRARY( SixB SixBSrc.c )
TARGET_LINK_LIBRARIES( SixB Four )

