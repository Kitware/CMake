# Microsoft specific config file 


SET (CMAKE_CXX_COMPILER  cl CACHE STRING 
     "Name of C++ compiler used.")

SET (CMAKE_CXX_FLAGS_RELEASE "/MD /O2" CACHE STRING
     "Flags used by the compiler during release builds (/MD /Ob1 /Oi /Ot /Oy /Gs will produce slightly less optimized but smaller files)")

SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MD /Zi /O2" CACHE STRING
     "Flags used by the compiler during Release with Debug Info builds")

SET (CMAKE_CXX_FLAGS_MINSIZEREL "/MD /O1" CACHE STRING
     "Flags used by the compiler during release minsize builds")

SET (CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /Od /GZ" CACHE STRING
     "Flags used by the compiler during debug builds")

SET (CMAKE_CXX_FLAGS "/W3 /Zm1000 /GX /GR" CACHE STRING
     "Flags used by the compiler during all build types, /GX /GR are for exceptions and rtti in VC++, /Zm1000 increases the compiler's memory allocation to support ANSI C++/stdlib")

SET (CMAKE_USE_WIN32_THREADS 1 CACHE BOOL 
     "Use the win32 thread library")

SET (CMAKE_MAKE_PROGRAM "msdev" CACHE STRING 
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
WORDS_BIGENDIAN
HAVE_UNISTD_H
HAVE_LIMITS_H
CMAKE_CXX_COMPILER
CMAKE_CXX_FLAGS_RELEASE
CMAKE_CXX_FLAGS_RELWITHDEBINFO
CMAKE_CXX_FLAGS_MINSIZEREL
CMAKE_CXX_FLAGS_DEBUG
CMAKE_USE_WIN32_THREADS
CMAKE_MAKE_PROGRAM
)


