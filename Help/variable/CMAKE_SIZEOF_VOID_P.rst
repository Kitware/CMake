CMAKE_SIZEOF_VOID_P
-------------------

Size of a void pointer.

This is set to the size of a pointer on the machine, and is determined
by a try compile.  If a 64 bit size is found, then the library search
path is modified to look for 64 bit libraries first.
