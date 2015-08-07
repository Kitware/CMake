CMAKE_LINK_SEARCH_START_STATIC
------------------------------

Assume the linker looks for static libraries by default.

Some linkers support switches such as -Bstatic and -Bdynamic to
determine whether to use static or shared libraries for -lXXX options.
CMake uses these options to set the link type for libraries whose full
paths are not known or (in some cases) are in implicit link
directories for the platform.  By default the linker search type is
assumed to be -Bdynamic at the beginning of the library list.  This
property switches the assumption to -Bstatic.  It is intended for use
when linking an executable statically (e.g.  with the GNU -static
option).

This variable is used to initialize the target property
LINK_SEARCH_START_STATIC for all targets.

See also CMAKE_LINK_SEARCH_END_STATIC.
