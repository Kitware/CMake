# 
# CMakeLocal.make.in should be in the directory where you run configure
# in, which need not be the source directory
#
SET (WORDS_BIGENDIAN )
SET (HAVE_LIMITS_H   1)
SET (HAVE_UNISTD_H   1)
SET (CXX  VC++60 )
SET (CMAKE_CXX_FLAGS_RELEASE "/MD /O2" CACHE STRING
        "Flags used by the compiler during release builds")
SET (CMAKE_CXX_FLAGS_MINSIZEREL "/MD /O1" CACHE STRING
        "Flags used by the compiler during release minsize builds")
SET (CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /Od /GZ" CACHE STRING
        "Flags used by the compiler during debug builds")
SET (CMAKE_CXX_FLAGS "/W3 /Zm1000 /GX /GR" CACHE STRING
        "Flags used by the compiler during all build types, /GX /GR are for exceptions and rtti in VC++")
