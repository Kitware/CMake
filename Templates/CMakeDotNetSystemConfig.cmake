# Microsoft specific config file 

SET (CMAKE_CXX_COMPILER  cl CACHE STRING 
     "Name of C++ compiler used.")

SET (CMAKE_CXX_FLAGS "/W3 /Zm1000 " CACHE STRING
     "Flags used by the compiler during all build types, /Zm1000 increases the compiler's memory allocation to support ANSI C++/stdlib")

SET (CMAKE_EXTRA_LINK_FLAGS "/STACK:10000000" CACHE STRING
     "Extra flags added to the link line for creation of exe and dlls.")

SET (CMAKE_USE_WIN32_THREADS 1 CACHE BOOL 
     "Use the win32 thread library")

SET (CMAKE_MAKE_PROGRAM "devenv" CACHE STRING 
     "Program used to build from dsp files.")

# We will hardcode them for now. Make sure to fix that in the future
SET (CMAKE_SIZEOF_INT       4   CACHE INTERNAL "Size of int data type")
SET (CMAKE_SIZEOF_LONG      4   CACHE INTERNAL "Size of long data type")
SET (CMAKE_SIZEOF_VOID_P    4   CACHE INTERNAL "Size of void* data type")
SET (CMAKE_SIZEOF_CHAR      1   CACHE INTERNAL "Size of char data type")
SET (CMAKE_SIZEOF_SHORT     2   CACHE INTERNAL "Size of short data type")
SET (CMAKE_SIZEOF_FLOAT     4   CACHE INTERNAL "Size of float data type")
SET (CMAKE_SIZEOF_DOUBLE    8   CACHE INTERNAL "Size of double data type")

# The following variables are advanced 

MARK_AS_ADVANCED(
CMAKE_CXX_COMPILER
CMAKE_USE_WIN32_THREADS
CMAKE_MAKE_PROGRAM
CMAKE_EXTRA_LINK_FLAGS
)


