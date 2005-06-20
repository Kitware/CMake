SET(CMAKE_LINK_LIBRARY_SUFFIX "")  

SET(CMAKE_STATIC_LIBRARY_PREFIX "lib")
SET(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
SET(CMAKE_SHARED_LIBRARY_PREFIX "lib")          # lib
SET(CMAKE_SHARED_LIBRARY_SUFFIX ".so")          # .so
SET(CMAKE_DL_LIBS "-ldl")
SET(CMAKE_SHARED_LIBRARY_C_FLAGS "")            # -pic 
SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-shared")       # -shared
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")         # +s, flag for exe link to use shared lib
SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "")       # -rpath
SET(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP "")   # : or empty
SET(CMAKE_INCLUDE_FLAG_C "-I")       # -I
SET(CMAKE_INCLUDE_FLAG_C_SEP "")     # , or empty
SET(CMAKE_LIBRARY_PATH_FLAG "-L")
SET(CMAKE_LINK_LIBRARY_FLAG "-l")
IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_SHARED_LIBRARY_C_FLAGS "-fPIC")     # -pic 
ENDIF(CMAKE_COMPILER_IS_GNUCC)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "-fPIC")   # -pic
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

SET (CMAKE_SKIP_RPATH "NO" CACHE BOOL
     "If set, runtime paths are not added when using shared libraries.")

   
SET(CMAKE_VERBOSE_MAKEFILE FALSE CACHE BOOL "If this value is on, makefiles will be generated without the .SILENT directive, and all commands will be echoed to the console during the make.  This is useful for debugging only. With Visual Studio IDE projects all commands are done without /nologo.") 

# Choose a default install prefix for this platform.
IF(UNIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local"
    CACHE PATH "Install path prefix, prepended onto install directories.")
ELSE(UNIX)
  IF("$ENV{SystemDrive}" MATCHES "^$")
    SET(CMAKE_GENERIC_SYSTEM_DRIVE "C:")
  ELSE("$ENV{SystemDrive}" MATCHES "^$")
    SET(CMAKE_GENERIC_SYSTEM_DRIVE "$ENV{SystemDrive}")
  ENDIF("$ENV{SystemDrive}" MATCHES "^$")
  SET(CMAKE_INSTALL_PREFIX
    "${CMAKE_GENERIC_SYSTEM_DRIVE}/Program Files/${PROJECT_NAME}"
    CACHE PATH "Install path prefix, prepended onto install directories.")
  SET(CMAKE_GENERIC_SYSTEM_DRIVE)
  MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)
ENDIF(UNIX)

MARK_AS_ADVANCED(
  CMAKE_SKIP_RPATH
  CMAKE_VERBOSE_MAKEFILE
)

# always include the gcc compiler information
INCLUDE(Platform/gcc)
