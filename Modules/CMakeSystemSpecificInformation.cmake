# this module sets system information like how to build libraries
# before including this file the system, C, and C++ compilers must
# have already been determined.
# This file first sets default variables that can be used for most
# makefiles.  Next, it will include a system specific file.  Finally,
# it will optionally include a system and compiler specific file that
# can be used to override any of this information.


# 1. set default values that will work for most system
SET(CMAKE_CXX_CREATE_SHARED_LIBRARY
    "${CMAKE_CXX_COMPILE} ${CMAKE_SHARED_LIBRARY_CREATE_FLAGS} ${CMAKE_CXX_LINK_SHARED_OUT_FLAG} <TARGET> <OBJECTS>")

SET(CMAKE_CXX_CREATE_AR_LIBRARY
    "${CMAKE_AR} ${CMAKE_AR_FLAGS} <TARGET> <OBJECTS>")

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
  INCLUDE(${CMAKE_ROOT}/Modules/${CMAKE_SYSTEM_NAME}-${CMAKE_C_COMPILER}.cmake OPTIONAL)
ENDIF(CMAKE_C_COMPILER)
IF(CMAKE_CXX_COMPILER)
  INCLUDE(${CMAKE_ROOT}/Modules/${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER}.cmake
          OPTIONAL)
ENDIF(CMAKE_CXX_COMPILER)
