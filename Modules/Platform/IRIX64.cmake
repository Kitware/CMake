SET(CMAKE_DL_LIBS "")
SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-shared -rdata_shared")
SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "-shared -rdata_shared") 
SET(CMAKE_SHARED_LIBRARY_RUNTIME_FLAG "-Wl,-rpath,")       # -rpath
SET(CMAKE_SHARED_LIBRARY_RUNTIME_FLAG_SEP "")   # : or empty
IF(NOT CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_CREATE_STATIC_LIBRARY
      "<CMAKE_CXX_COMPILER> -ar -o <TARGET> <OBJECTS>")
  SET(CMAKE_ANSI_CXXFLAGS -LANG:std)
  SET (CMAKE_CXX_FLAGS "" CACHE STRING
     "Flags used by the compiler during all build types.")

  SET (CMAKE_CXX_FLAGS_DEBUG "-g" CACHE STRING
     "Flags used by the compiler during debug builds.")

  SET (CMAKE_CXX_FLAGS_MINSIZEREL "-O3" CACHE STRING
     "Flags used by the compiler during release minsize builds.")

  SET (CMAKE_CXX_FLAGS_RELEASE "-O2" CACHE STRING
     "Flags used by the compiler during release builds (/MD /Ob1 /Oi /Ot /Oy /Gs will produce slightly less optimized but smaller files).")

  SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2" CACHE STRING
     "Flags used by the compiler during Release with Debug Info builds.")


  SET (CMAKE_C_FLAGS "" CACHE STRING
     "Flags for C compiler.")
ELSE(NOT CMAKE_COMPILER_IS_GNUCXX)
  INCLUDE(${CMAKE_ROOT}/Modules/Platform/gcc.cmake)
ENDIF(NOT CMAKE_COMPILER_IS_GNUCXX)

