# this module sets system information like how to build libraries
# before including this file the system, C, and C++ compilers must
# have already been determined.
# This file first sets default variables that can be used for most
# makefiles.  Next, it will include a system specific file.  Finally,
# it will optionally include a system and compiler specific file that
# can be used to override any of this information.
# For debugging new systems, and expert users, if the
# CMAKE_USER_MAKE_RULES_OVERRIDE is set to a file name, that
# file will be included last, and can override any variable


# 1. set default values that will work for most system
SET(CMAKE_STATIC_LIBRARY_PREFIX "lib")
SET(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
SET(CMAKE_SHARED_LIBRARY_C_FLAGS "")            # -pic 
SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "")          # -pic
SET(CMAKE_SHARED_LIBRARY_CREATE_FLAGS "")       # -shared
SET(CMAKE_SHARED_LIBRARY_PREFIX "lib")          # lib
SET(CMAKE_SHARED_LIBRARY_SUFFIX ".so")          # .so
SET(CMAKE_SHARED_LIBRARY_LINK_FLAGS "")         # +s, flag for exe link to use shared lib
SET(CMAKE_SHARED_LIBRARY_RUNTIME_FLAG "")       # -rpath
SET(CMAKE_SHARED_LIBRARY_RUNTIME_FLAG_SEP "")   # : or empty
SET(CMAKE_LIBRARY_PATH_FLAG "-L")
SET(CMAKE_LINK_LIBRARY_FLAG "-l")
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

# 3. include optional systemname-compiler.cmake files
IF(CMAKE_C_COMPILER)
  GET_FILENAME_COMPONENT(CMAKE_BASE_NAME ${CMAKE_C_COMPILER} NAME_WE)
  # since the gnu compiler has several names force gcc
  IF(CMAKE_COMPILER_IS_GNUGCC)
     SET(CMAKE_BASE_NAME gcc)
  ENDIF(CMAKE_COMPILER_IS_GNUGCC)
  SET(CMAKE_SYSTEM_AND_C_COMPILER_INFO_FILE 
      ${CMAKE_ROOT}/Modules/${CMAKE_SYSTEM_NAME}-${CMAKE_BASE_NAME}.cmake)
  INCLUDE(${CMAKE_SYSTEM_AND_C_COMPILER_INFO_FILE} OPTIONAL)
ENDIF(CMAKE_C_COMPILER)
IF(CMAKE_CXX_COMPILER)
  GET_FILENAME_COMPONENT(CMAKE_BASE_NAME ${CMAKE_CXX_COMPILER} NAME_WE)
  # since the gnu compiler has several names force gcc
  IF(CMAKE_COMPILER_IS_GNUGXX)
     SET(CMAKE_BASE_NAME g++)
  ENDIF(CMAKE_COMPILER_IS_GNUGXX)
  SET(CMAKE_SYSTEM_AND_CXX_COMPILER_INFO_FILE
      ${CMAKE_ROOT}/Modules/${CMAKE_SYSTEM_NAME}-${CMAKE_BASE_NAME}.cmake)
  INCLUDE(${CMAKE_SYSTEM_AND_CXX_COMPILER_INFO_FILE} OPTIONAL)
ENDIF(CMAKE_CXX_COMPILER)


# for most systems a module is the same as a shared library
# so unless the variable CMAKE_MODULE_EXISTS is set just
# copy the values from the LIBRARY variables
IF(NOT CMAKE_MODULE_EXISTS)
  SET(CMAKE_SHARED_MODULE_C_FLAGS ${CMAKE_SHARED_LIBRARY_C_FLAGS)
  SET(CMAKE_SHARED_MODULE_CXX_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})
  SET(CMAKE_SHARED_MODULE_CREATE_FLAGS ${CMAKE_SHARED_LIBRARY_CREATE_FLAGS})
  SET(CMAKE_SHARED_MODULE_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
  SET(CMAKE_SHARED_MODULE_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
  SET(CMAKE_SHARED_MODULE_LINK_FLAGS ${CMAKE_SHARED_LIBRARY_LINK_FLAGS})
  SET(CMAKE_SHARED_MODULE_RUNTIME_FLAG ${CMAKE_SHARED_LIBRARY_RUNTIME_FLAG})
  SET(CMAKE_SHARED_MODULE_RUNTIME_FLAG_SEP ${CMAKE_SHARED_MODULE_RUNTIME_FLAG_SEP})
ENDIF(NOT CMAKE_MODULE_EXISTS)

# include default rules that work for most unix like systems and compilers
# this file will not set anything if it is already set
INCLUDE(${CMAKE_ROOT}/Modules/CMakeDefaultMakeRuleVariables.cmake)

IF(CMAKE_USER_MAKE_RULES_OVERRIDE)
   INCLUDE(${CMAKE_USER_MAKE_RULES_OVERRIDE})
ENDIF(CMAKE_USER_MAKE_RULES_OVERRIDE)
