SET(CMAKE_SHARED_LIBRARY_SUFFIX ".sl")          # .so
SET(CMAKE_DL_LIBS "-ldld")

SET(CMAKE_SHARED_LIBRARY_RUNTIME_FLAG_SEP ":")   # : or empty

# C compiler
IF(CMAKE_COMPILER_IS_GNUCC)
  # gnu gcc
  SET(CMAKE_SHARED_LIBRARY_C_FLAGS "-fPIC")            # -pic 
  SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-shared -Wl,-E -Wl,-b")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_FLAGS "-Wl,+s")         # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_FLAG "-Wl,+b")       # -rpath
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_FLAG_SEP ":")   # : or empty
  SET(CMAKE_SHARED_LIBRARY_C_FLAGS "-fPIC")     # -pic 
ELSE(CMAKE_COMPILER_IS_GNUCC)
  # hp cc
  SET(CMAKE_ANSI_CFLAGS "-Ae -Aa")
  # use ld directly to create shared libraries for hp cc
  SET(CMAKE_C_CREATE_SHARED_LIBRARY
      "ld <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
  SET(CMAKE_SHARED_LIBRARY_C_FLAGS "+Z")            # -pic 
  SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-E -b -L/usr/lib")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_FLAGS "+s")         # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_FLAG "+b")       # -rpath
ENDIF(CMAKE_COMPILER_IS_GNUCC)

# CXX compiler
IF(CMAKE_COMPILER_IS_GNUCXX) 
  INCLUDE(${CMAKE_ROOT}/Modules/Platform/gcc.cmake)
  # for gnu C++
  SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "-fPIC")            # -pic 
  SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "-shared -Wl,-E -Wl,-b")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-Wl,+s")     # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG "-Wl,+b")       # -rpath
  SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "-fPIC")     # -pic 
ELSE(CMAKE_COMPILER_IS_GNUCXX)
  # for hp aCC
  SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "+Z")            # -pic 
  SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "+Z -Wl,-E -b -L/usr/lib")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-Wl,+s")         # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG "-Wl,+b")       # -rpath
  SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "-fPIC")     # -pic 

  IF(NOT CMAKE_CXX_FLAGS)
    SET (CMAKE_CXX_FLAGS "")
    SET (CMAKE_CXX_FLAGS_DEBUG "-g")
    SET (CMAKE_CXX_FLAGS_MINSIZEREL "-O3")
    SET (CMAKE_CXX_FLAGS_RELEASE "-O2")
    SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g")
    SET (CMAKE_C_FLAGS "")
  ENDIF(NOT CMAKE_CXX_FLAGS)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)
