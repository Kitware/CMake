# microsoft specific config file 
SET (WORDS_BIGENDIAN )
SET (HAVE_LIMITS_H   1)
SET (HAVE_UNISTD_H   1)
SET (CMAKE_CXX_COMPILER  VC++60 CACHE STRING 
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
SET (CMAKE_USE_WIN32_THREADS 1 CACHE BOOL "Use the win32 thread library")
SET (CMAKE_MAKE_PROGRAM   "msdev" CACHE STRING "Program used to build from dsp files.")
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