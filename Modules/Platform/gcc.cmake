SET (CMAKE_CXX_FLAGS "" CACHE STRING
     "Flags used by the compiler during all build types.")

SET (CMAKE_CXX_FLAGS_DEBUG "-g" CACHE STRING
     "Flags used by the compiler during debug builds.")

SET (CMAKE_CXX_FLAGS_MINSIZEREL "-O3" CACHE STRING
     "Flags used by the compiler during release minsize builds.")

SET (CMAKE_CXX_FLAGS_RELEASE "-O2" CACHE STRING
     "Flags used by the compiler during release builds (/MD /Ob1 /Oi /Ot /Oy /Gs will produce slightly less optimized but smaller files).")

SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g" CACHE STRING
     "Flags used by the compiler during Release with Debug Info builds.")


SET (CMAKE_C_FLAGS "" CACHE STRING
     "Flags for C compiler.")
