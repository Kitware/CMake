# Microsoft specific config file 

SET (CMAKE_CXX_COMPILER  cl CACHE STRING 
     "Name of C++ compiler used.")

SET (CMAKE_CXX_FLAGS "/Zm1000 " CACHE STRING
     "Flags used by the compiler during all build types, /Zm1000 increases the compiler's memory allocation to support ANSI C++/stdlib, /W3 sets the warning level to 3")

SET (CMAKE_CXX_STACK_SIZE "10000000" CACHE STRING
     "Size of stack for programs.")

SET (CMAKE_CXX_WARNING_LEVEL "3" CACHE STRING
     "Size of stack for programs.")

SET (CMAKE_CXX_USE_RTTI 1 CACHE BOOL 
     "Compile CXX code with run time type information.")

SET (CMAKE_USE_WIN32_THREADS 1 CACHE BOOL 
     "Use the win32 thread library")

SET (CMAKE_MAKE_PROGRAM "devenv" CACHE STRING 
     "Program used to build from dsp files.")

SET (CMAKE_CONFIGURATION_TYPES "Debug Release MinSizeRel RelWithDebInfo" CACHE STRING 
     "Space separated list of supported configuration types, only supports Debug, Release, MinSizeRel, and RelWithDebInfo, anything else will be ignored.")

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
CMAKE_CXX_USE_RTTI
CMAKE_CXX_COMPILER
CMAKE_CXX_STACK_SIZE
CMAKE_CXX_WARNING_LEVEL
CMAKE_USE_WIN32_THREADS
CMAKE_MAKE_PROGRAM
CMAKE_EXTRA_LINK_FLAGS
)


