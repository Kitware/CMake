SET(CMAKE_SHARED_LIBRARY_PREFIX "lib")          # lib
SET(CMAKE_SHARED_LIBRARY_SUFFIX ".so")          # .so
SET(CMAKE_DL_LIBS "-lld")

# RPATH support on AIX is called libpath.  By default the runtime
# libpath is paths specified by -L followed by /usr/lib and /lib.  In
# order to prevent the -L paths from being used we must force use of
# -Wl,-blibpath:/usr/lib:/lib whether RPATH support is on or not.
# When our own RPATH is to be added it may be inserted before the
# "always" paths.
SET(CMAKE_PLATFORM_REQUIRED_RUNTIME_PATH /usr/lib /lib)
SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,-blibpath:")
SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")

# Files named "libfoo.a" may actually be shared libraries.
SET_PROPERTY(GLOBAL PROPERTY TARGET_ARCHIVES_MAY_BE_SHARED_LIBS 1)

# CXX Compiler
IF(CMAKE_COMPILER_IS_GNUCXX) 
  SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "-shared -Wl,-G")       # -shared
ELSE(CMAKE_COMPILER_IS_GNUCXX) 
  SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "-G -Wl,-brtl,-bnoipath")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-Wl,-brtl,-bnoipath,-bexpall")  # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS " ")
  SET(CMAKE_SHARED_MODULE_CXX_FLAGS  " ")
  SET (CMAKE_CXX_FLAGS_DEBUG_INIT "-g")
  SET (CMAKE_CXX_FLAGS_RELEASE_INIT "-O -DNDEBUG") 
  SET (CMAKE_CXX_FLAGS_MINSIZEREL_INIT "-O -DNDEBUG")
  SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-g")
ENDIF(CMAKE_COMPILER_IS_GNUCXX) 

# C Compiler
IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-shared -Wl,-G")       # -shared
ELSE(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-G -Wl,-brtl,-bnoipath")  # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-Wl,-brtl,-bnoipath,-bexpall")  # +s, flag for exe link to use shared lib
  SET(CMAKE_SHARED_LIBRARY_C_FLAGS " ")
  SET(CMAKE_SHARED_MODULE_C_FLAGS  " ")
  SET (CMAKE_C_FLAGS_DEBUG_INIT "-g")
  SET (CMAKE_C_FLAGS_RELEASE_INIT "-O -DNDEBUG") 
  SET (CMAKE_C_FLAGS_MINSIZEREL_INIT "-O -DNDEBUG")
  SET (CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "-g")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

IF(NOT CMAKE_COMPILER_IS_GNUCC)
  SET (CMAKE_C_CREATE_PREPROCESSED_SOURCE "<CMAKE_C_COMPILER> <DEFINES> <FLAGS> -E <SOURCE> > <PREPROCESSED_SOURCE>")
  SET (CMAKE_C_CREATE_ASSEMBLY_SOURCE "<CMAKE_C_COMPILER> <DEFINES> <FLAGS> -S <SOURCE> -o <ASSEMBLY_SOURCE>")
ENDIF(NOT CMAKE_COMPILER_IS_GNUCC)

IF(NOT CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_COMPILE_OBJECT
    "<CMAKE_CXX_COMPILER> -+ <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>")
  SET (CMAKE_CXX_CREATE_PREPROCESSED_SOURCE "<CMAKE_CXX_COMPILER> <DEFINES> <FLAGS> -E <SOURCE> > <PREPROCESSED_SOURCE>")
  SET (CMAKE_CXX_CREATE_ASSEMBLY_SOURCE "<CMAKE_CXX_COMPILER> <DEFINES> <FLAGS> -S <SOURCE> -o <ASSEMBLY_SOURCE>")
ENDIF(NOT CMAKE_COMPILER_IS_GNUCXX)


# since .a can be a static or shared library on AIX, we can not do this.
# at some point if we wanted it, we would have to figure out if a .a is
# static or shared, then we could add this back:

# Initialize C link type selection flags.  These flags are used when
# building a shared library, shared module, or executable that links
# to other libraries to select whether to use the static or shared
# versions of the libraries.
#FOREACH(type SHARED_LIBRARY SHARED_MODULE EXE)
#  SET(CMAKE_${type}_LINK_STATIC_C_FLAGS "-bstatic")
#  SET(CMAKE_${type}_LINK_DYNAMIC_C_FLAGS "-bdynamic")
#ENDFOREACH(type)

INCLUDE(Platform/UnixPaths)
