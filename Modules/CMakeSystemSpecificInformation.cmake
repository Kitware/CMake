# this module sets system information like how to build libraries
# before including this file the system, C, and C++ compilers must
# have already been determined.
# This file first sets default variables that can be used for most
# makefiles.  Next, it will include a system specific file.  Finally,
# it will optionally include a system and compiler specific file that
# can be used to override any of this information.


# 1. set default values that will work for most system
SET(CMAKE_SHARED_LIBRARY_C_FLAGS "")            # -pic 
SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "")          # -pic
# for gnu compiler always 
  
SET(CMAKE_SHARED_LIBRARY_CREATE_FLAGS "")       # -shared
SET(CMAKE_SHARED_LIBRARY_PREFIX "lib")          # lib
SET(CMAKE_SHARED_LIBRARY_SUFFIX ".so")          # .so
SET(CMAKE_SHARED_LIBRARY_LINK_FLAGS "")         # +s, or some flag that an exe needs to use a shared lib
SET(CMAKE_SHARED_LIBRARY_RUNTIME_FLAG "")       # -rpath
SET(CMAKE_SHARED_LIBRARY_RUNTIME_FLAG_SEP "")   # : or empty

IF(CMAKE_COMPILER_IS_GNUGXX)
  SET(CMAKE_SHARED_LIBRARY_C_FLAGS "-fPIC")     # -pic 
  SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "-fPIC")   # -pic
ENDIF(CMAKE_COMPILER_IS_GNUGXX)


SET (CMAKE_SKIP_RPATH "NO" CACHE BOOL
     "If set, runtime paths are not added when using shared libraries.")

# 2. now include SystemName.cmake file to set the system specific information
SET(CMAKE_SYSTEM_INFO_FILE ${CMAKE_ROOT}/Modules/${CMAKE_SYSTEM_NAME}.cmake)
IF(EXISTS ${CMAKE_SYSTEM_INFO_FILE})
  INCLUDE(${CMAKE_SYSTEM_INFO_FILE})
ELSE(EXISTS ${CMAKE_SYSTEM_INFO_FILE})
  MESSAGE("System is unknown to cmake, create:\n${CMAKE_SYSTEM_INFO_FILE}"
          " to use this system, please send your config file to "
          "cmake@www.cmake.org so it can be added to cmake"")
ENDIF(EXISTS ${CMAKE_SYSTEM_INFO_FILE})



# Set up default values for things that have not been set either
# in SYSTEM.cmake or SYSTEM-compiler.cmake

IF(NOT CMAKE_CXX_CREATE_SHARED_LIBRARY)
  SET(CMAKE_CXX_CREATE_SHARED_LIBRARY
      "${CMAKE_CXX_COMPILE} ${CMAKE_SHARED_LIBRARY_CREATE_FLAGS} "
      "${CMAKE_CXX_LINK_SHARED_OUT_FLAG} <TARGET> <OBJECTS>")
ENDIF(NOT CMAKE_CXX_CREATE_SHARED_LIBRARY)


IF(NOT CMAKE_CXX_CREATE_STATIC_LIBRARY)
  SET(CMAKE_CXX_CREATE_STATIC_LIBRARY
      "${CMAKE_CXX_AR} ${CMAKE_AR_FLAGS} <TARGET> <OBJECTS>")
ENDIF(NOT CMAKE_CXX_CREATE_STATIC_LIBRARY)


IF(NOT CMAKE_CXX_COMPILE)
  SET(CMAKE_CXX_COMPILE
    "${CMAKE_CXX_COMPILER} -o <OBJECT> ${CMAKE_CXX_FLAGS} -c <SOURCE>")
ENDIF(NOT CMAKE_CXX_COMPILE)


# 3. include optional systemname-compiler.cmake files
IF(CMAKE_C_COMPILER)
  INCLUDE(${CMAKE_ROOT}/Modules/${CMAKE_SYSTEM_NAME}-${CMAKE_C_COMPILER}.cmake OPTIONAL)
ENDIF(CMAKE_C_COMPILER)
IF(CMAKE_CXX_COMPILER)
  INCLUDE(${CMAKE_ROOT}/Modules/${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER}.cmake
          OPTIONAL)
ENDIF(CMAKE_CXX_COMPILER)
