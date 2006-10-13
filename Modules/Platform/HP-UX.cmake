SET(CMAKE_SHARED_LIBRARY_SUFFIX ".sl")          # .so
SET(CMAKE_DL_LIBS "dld")
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".sl" ".so" ".a")

SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")   # : or empty

# fortran
IF(CMAKE_COMPILER_IS_GNUG77)
  SET(CMAKE_SHARED_LIBRARY_Fortran_FLAGS "-fPIC")            # -pic 
  SET(CMAKE_SHARED_LIBRARY_CREATE_Fortran_FLAGS "-shared -Wl,-E -Wl,-b")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-Wl,+s -Wl,-E")  # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,+b")       # -rpath
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")   # : or empty
  SET(CMAKE_SHARED_LIBRARY_SONAME_Fortran_FLAG "-Wl,+h")
  SET(CMAKE_SHARED_LIBRARY_Fortran_FLAGS "-fPIC")     # -pic 
ELSE(CMAKE_COMPILER_IS_GNUG77)
  # use ld directly to create shared libraries for hp cc
  SET(CMAKE_Fortran_CREATE_SHARED_LIBRARY
      "ld <CMAKE_SHARED_LIBRARY_CREATE_Fortran_FLAGS> <CMAKE_SHARED_LIBRARY_SONAME_Fortran_FLAG><TARGET_SONAME> <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
  SET(CMAKE_SHARED_LIBRARY_Fortran_FLAGS "+Z")            # -pic 
  SET(CMAKE_SHARED_LIBRARY_CREATE_Fortran_FLAGS "-E -b -L/usr/lib")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-Wl,+s -Wl,-E")  # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "+b")       # -rpath
  SET(CMAKE_SHARED_LIBRARY_SONAME_Fortran_FLAG "+h")
ENDIF(CMAKE_COMPILER_IS_GNUG77)
# C compiler
IF(CMAKE_COMPILER_IS_GNUCC)
  # gnu gcc
  SET(CMAKE_SHARED_LIBRARY_C_FLAGS "-fPIC")            # -pic 
  SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-shared -Wl,-E -Wl,-b")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-Wl,+s -Wl,-E")  # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,+b")       # -rpath
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")   # : or empty
  SET(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-Wl,+h")
  SET(CMAKE_SHARED_LIBRARY_C_FLAGS "-fPIC")     # -pic 
ELSE(CMAKE_COMPILER_IS_GNUCC)
  # hp cc
  SET(CMAKE_ANSI_CFLAGS "-Aa -Ae")
  # use ld directly to create shared libraries for hp cc
  SET(CMAKE_C_CREATE_SHARED_LIBRARY
      "ld <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> <CMAKE_SHARED_LIBRARY_SONAME_C_FLAG><TARGET_SONAME> <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
  SET(CMAKE_SHARED_LIBRARY_C_FLAGS "+Z")            # -pic 
  SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-E -b -L/usr/lib")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-Wl,+s -Wl,-E")  # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "+b")       # -rpath
  SET(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "+h")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

# CXX compiler
IF(CMAKE_COMPILER_IS_GNUCXX) 
  # for gnu C++
  SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "-fPIC")            # -pic 
  SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "-shared -Wl,-E -Wl,-b")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-Wl,+s -Wl,-E")  # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG "-Wl,+b")       # -rpath
  SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "-fPIC")     # -pic 
  SET(CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG "-Wl,+h")
ELSE(CMAKE_COMPILER_IS_GNUCXX)
  # for hp aCC
  SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "+Z")            # -pic 
  SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "+Z -Wl,-E -b -L/usr/lib")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-Wl,+s -Wl,-E")  # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG "-Wl,+b")       # -rpath
  SET(CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG "-Wl,+h")
  SET (CMAKE_CXX_FLAGS_INIT "")
  SET (CMAKE_CXX_FLAGS_DEBUG_INIT "-g")
  SET (CMAKE_CXX_FLAGS_MINSIZEREL_INIT "+O3 -DNDEBUG")
  SET (CMAKE_CXX_FLAGS_RELEASE_INIT "+O2 -DNDEBUG")
  SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-g")
  SET (CMAKE_C_FLAGS_INIT "")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)
# set flags for gcc support
INCLUDE(Platform/UnixPaths)

IF(NOT CMAKE_COMPILER_IS_GNUCC)
  SET (CMAKE_C_CREATE_PREPROCESSED_SOURCE "<CMAKE_C_COMPILER> <FLAGS> -E <SOURCE> > <PREPROCESSED_SOURCE>")
  SET (CMAKE_C_CREATE_ASSEMBLY_SOURCE "<CMAKE_C_COMPILER> <FLAGS> -S <SOURCE> -o <ASSEMBLY_SOURCE>")
ENDIF(NOT CMAKE_COMPILER_IS_GNUCC)

IF(NOT CMAKE_COMPILER_IS_GNUCXX)
  SET (CMAKE_CXX_CREATE_PREPROCESSED_SOURCE "<CMAKE_CXX_COMPILER> <FLAGS> -E <SOURCE> > <PREPROCESSED_SOURCE>")
  SET (CMAKE_CXX_CREATE_ASSEMBLY_SOURCE
    "<CMAKE_CXX_COMPILER> <FLAGS> -S <SOURCE>"
    "mv `basename \"<SOURCE>\" | sed 's/\\.[^./]*$$//'`.s <ASSEMBLY_SOURCE>"
    "rm -f `basename \"<SOURCE>\" | sed 's/\\.[^./]*$$//'`.o"
    )
ENDIF(NOT CMAKE_COMPILER_IS_GNUCXX)

# Initialize C and CXX link type selection flags.  These flags are
# used when building a shared library, shared module, or executable
# that links to other libraries to select whether to use the static or
# shared versions of the libraries.  Note that C modules and shared
# libs are built using ld directly so we leave off the "-Wl," portion.
FOREACH(type SHARED_LIBRARY SHARED_MODULE)
  SET(CMAKE_${type}_LINK_STATIC_C_FLAGS "-a archive")
  SET(CMAKE_${type}_LINK_DYNAMIC_C_FLAGS "-a default")
ENDFOREACH(type)
FOREACH(type EXE)
  SET(CMAKE_${type}_LINK_STATIC_C_FLAGS "-Wl,-a,archive")
  SET(CMAKE_${type}_LINK_DYNAMIC_C_FLAGS "-Wl,-a,default")
ENDFOREACH(type)
FOREACH(type SHARED_LIBRARY SHARED_MODULE EXE)
  SET(CMAKE_${type}_LINK_STATIC_CXX_FLAGS "-Wl,-a,archive")
  SET(CMAKE_${type}_LINK_DYNAMIC_CXX_FLAGS "-Wl,-a,default")
ENDFOREACH(type)

