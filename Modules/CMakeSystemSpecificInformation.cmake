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

# suffix that needs to be added onto a library to link it .lib on windows
# not used on most unix systems
SET(CMAKE_LINK_LIBRARY_SUFFIX "")  

SET(CMAKE_STATIC_LIBRARY_PREFIX "lib")
SET(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
SET(CMAKE_SHARED_LIBRARY_PREFIX "lib")          # lib
SET(CMAKE_SHARED_LIBRARY_SUFFIX ".so")          # .so
SET(CMAKE_DL_LIBS "-ldl")
SET(CMAKE_SHARED_LIBRARY_C_FLAGS "")            # -pic 
SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-shared")       # -shared
SET(CMAKE_SHARED_LIBRARY_LINK_FLAGS "")         # +s, flag for exe link to use shared lib
SET(CMAKE_SHARED_LIBRARY_RUNTIME_FLAG "")       # -rpath
SET(CMAKE_SHARED_LIBRARY_RUNTIME_FLAG_SEP "")   # : or empty
SET(CMAKE_LIBRARY_PATH_FLAG "-L")
SET(CMAKE_LINK_LIBRARY_FLAG "-l")
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_SHARED_LIBRARY_C_FLAGS "-fPIC")     # -pic 
  SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS "-fPIC")   # -pic
ENDIF(CMAKE_COMPILER_IS_GNUCXX)


SET (CMAKE_SKIP_RPATH "NO" CACHE BOOL
     "If set, runtime paths are not added when using shared libraries.")
MARK_AS_ADVANCED(CMAKE_SKIP_RPATH)
# 2. now include SystemName.cmake file to set the system specific information
SET(CMAKE_SYSTEM_INFO_FILE ${CMAKE_ROOT}/Modules/Platform/${CMAKE_SYSTEM_NAME}.cmake)
IF(EXISTS ${CMAKE_SYSTEM_INFO_FILE})
  INCLUDE(${CMAKE_SYSTEM_INFO_FILE})
ELSE(EXISTS ${CMAKE_SYSTEM_INFO_FILE})
  MESSAGE("System is unknown to cmake, create:\n${CMAKE_SYSTEM_INFO_FILE}"
          " to use this system, please send your config file to "
          "cmake@www.cmake.org so it can be added to cmake")
  IF(EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
    CONFIGURE_FILE(${CMAKE_BINARY_DIR}/CMakeCache.txt
                   ${CMAKE_BINARY_DIR}/CopyOfCMakeCache.txt COPYONLY)
    MESSAGE("You CMakeCache.txt file was copied to CopyOfCMakeCache.txt. " 
            "Please send that file to cmake@www.cmake.org.")
  ENDIF(EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
ENDIF(EXISTS ${CMAKE_SYSTEM_INFO_FILE})

# 3. include optional systemname-compiler.cmake files
IF(CMAKE_C_COMPILER)
  GET_FILENAME_COMPONENT(CMAKE_BASE_NAME ${CMAKE_C_COMPILER} NAME_WE)
  # since the gnu compiler has several names force gcc
  IF(CMAKE_COMPILER_IS_GNUCC)
     SET(CMAKE_BASE_NAME gcc)
  ENDIF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_SYSTEM_AND_C_COMPILER_INFO_FILE 
      ${CMAKE_ROOT}/Modules/Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_BASE_NAME}.cmake)
  INCLUDE(${CMAKE_SYSTEM_AND_C_COMPILER_INFO_FILE} OPTIONAL)
ENDIF(CMAKE_C_COMPILER)
IF(CMAKE_CXX_COMPILER)
  GET_FILENAME_COMPONENT(CMAKE_BASE_NAME ${CMAKE_CXX_COMPILER} NAME_WE)
  # since the gnu compiler has several names force g++
  IF(CMAKE_COMPILER_IS_GNUCXX)
     SET(CMAKE_BASE_NAME g++)
  ENDIF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_SYSTEM_AND_CXX_COMPILER_INFO_FILE
      ${CMAKE_ROOT}/Modules/Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_BASE_NAME}.cmake)
  INCLUDE(${CMAKE_SYSTEM_AND_CXX_COMPILER_INFO_FILE} OPTIONAL)
ENDIF(CMAKE_CXX_COMPILER)


# for most systems a module is the same as a shared library
# so unless the variable CMAKE_MODULE_EXISTS is set just
# copy the values from the LIBRARY variables
IF(NOT CMAKE_MODULE_EXISTS)
  SET(CMAKE_SHARED_MODULE_C_FLAGS ${CMAKE_SHARED_LIBRARY_C_FLAGS})
  SET(CMAKE_SHARED_MODULE_CXX_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})
  SET(CMAKE_SHARED_MODULE_CREATE_C_FLAGS ${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS})
  SET(CMAKE_SHARED_MODULE_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
  SET(CMAKE_SHARED_MODULE_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
  SET(CMAKE_SHARED_MODULE_LINK_FLAGS ${CMAKE_SHARED_LIBRARY_LINK_FLAGS})
  SET(CMAKE_SHARED_MODULE_RUNTIME_FLAG ${CMAKE_SHARED_LIBRARY_RUNTIME_FLAG})
  SET(CMAKE_SHARED_MODULE_RUNTIME_FLAG_SEP ${CMAKE_SHARED_LIBRARY_RUNTIME_FLAG_SEP})
ENDIF(NOT CMAKE_MODULE_EXISTS)


# Create a set of shared library variable specific to C++
# For 90% of the systems, these are the same flags as the C versions
# so if these are not set just copy the flags from the c version
IF(NOT CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS)
  SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS ${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS})
ENDIF(NOT CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS)

IF(NOT CMAKE_SHARED_LIBRARY_CXX_FLAGS)
  SET(CMAKE_SHARED_LIBRARY_CXX_FLAGS ${CMAKE_SHARED_LIBRARY_C_FLAGS})
ENDIF(NOT CMAKE_SHARED_LIBRARY_CXX_FLAGS)

IF(NOT CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
  SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS ${CMAKE_SHARED_LIBRARY_LINK_FLAGS})
ENDIF(NOT CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

IF(NOT CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG)
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG ${CMAKE_SHARED_LIBRARY_RUNTIME_FLAG}) 
ENDIF(NOT CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG)

IF(NOT CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG_SEP)
  SET(CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG_SEP ${CMAKE_SHARED_LIBRARY_RUNTIME_FLAG_SEP})
ENDIF(NOT CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG_SEP)

# repeat for modules
IF(NOT CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS)
  SET(CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS ${CMAKE_SHARED_MODULE_CREATE_C_FLAGS})
ENDIF(NOT CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS)

IF(NOT CMAKE_SHARED_MODULE_CXX_FLAGS)
  SET(CMAKE_SHARED_MODULE_CXX_FLAGS ${CMAKE_SHARED_MODULE_C_FLAGS})
ENDIF(NOT CMAKE_SHARED_MODULE_CXX_FLAGS)

IF(NOT CMAKE_SHARED_MODULE_LINK_CXX_FLAGS)
  SET(CMAKE_SHARED_MODULE_LINK_CXX_FLAGS ${CMAKE_SHARED_MODULE_LINK_FLAGS})
ENDIF(NOT CMAKE_SHARED_MODULE_LINK_CXX_FLAGS)

IF(NOT CMAKE_SHARED_MODULE_RUNTIME_CXX_FLAG)
  SET(CMAKE_SHARED_MODULE_RUNTIME_CXX_FLAG ${CMAKE_SHARED_MODULE_RUNTIME_FLAG}) 
ENDIF(NOT CMAKE_SHARED_MODULE_RUNTIME_CXX_FLAG)

IF(NOT CMAKE_SHARED_MODULE_RUNTIME_CXX_FLAG_SEP)
  SET(CMAKE_SHARED_MODULE_RUNTIME_CXX_FLAG_SEP ${CMAKE_SHARED_MODULE_RUNTIME_FLAG_SEP})
ENDIF(NOT CMAKE_SHARED_MODULE_RUNTIME_CXX_FLAG_SEP)

# include default rules that work for most unix like systems and compilers
# this file will not set anything if it is already set
INCLUDE(${CMAKE_ROOT}/Modules/CMakeDefaultMakeRuleVariables.cmake)

IF(CMAKE_USER_MAKE_RULES_OVERRIDE)
   INCLUDE(${CMAKE_USER_MAKE_RULES_OVERRIDE})
ENDIF(CMAKE_USER_MAKE_RULES_OVERRIDE)

SET(CMAKE_VERBOSE_MAKEFILE FALSE CACHE BOOL "If this value is on, makefiles will be generated without the .SILENT directive, and all commands will be echoed to the console during the make.  This is useful for debugging only. With Visual Studio IDE projects all commands are done without /nologo.")

SET (CMAKE_INSTALL_PREFIX    /usr/local CACHE PATH 
     "Install path prefix, prepended onto install directories.")

# add the flags to the cache based
# on the initial values computed in the platform/*.cmake files
# use _INIT variables so that this only happens the first time
# and you can set these flags in the cmake cache
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_ENV_INIT} $ENV{CXXFLAGS} ${CMAKE_CXX_FLAGS_INIT}" CACHE STRING
     "Flags used by the compiler during all build types.")

SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS_ENV_INIT} $ENV{CFLAGS} ${CMAKE_C_FLAGS_INIT}" CACHE STRING
     "Flags for C compiler.")

SET (CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS_INIT} $ENV{LDFLAGS}
     CACHE STRING "Flags used by the linker.")

IF(NOT CMAKE_NOT_USING_CONFIG_FLAGS)
# default build type is none
  IF(NOT CMAKE_NO_BUILD_TYPE)
    SET (CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_INIT} CACHE STRING 
      "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
  ENDIF(NOT CMAKE_NO_BUILD_TYPE)

  SET (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG_INIT}" CACHE STRING
     "Flags used by the compiler during debug builds.")
  SET (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL_INIT}" CACHE STRING
      "Flags used by the compiler during release minsize builds.")
  SET (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE_INIT}" CACHE STRING
     "Flags used by the compiler during release builds (/MD /Ob1 /Oi /Ot /Oy /Gs will produce slightly less optimized but smaller files).")
  SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT}" CACHE STRING
     "Flags used by the compiler during Release with Debug Info builds.")
  SET (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG_INIT}" CACHE STRING
      "Flags used by the compiler during debug builds.")
  SET (CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL_INIT}" CACHE STRING
      "Flags used by the compiler during release minsize builds.")
  SET (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE_INIT}" CACHE STRING
     "Flags used by the compiler during release builds (/MD /Ob1 /Oi /Ot /Oy /Gs will produce slightly less optimized but smaller files).")
  SET (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO_INIT}" CACHE STRING
     "Flags used by the compiler during Release with Debug Info builds.")

  SET (CMAKE_EXE_LINKER_FLAGS_DEBUG ${CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT} CACHE STRING
     "Flags used by the linker during debug builds.")

  SET (CMAKE_EXE_LINKER_FLAGS_MINSIZEREL ${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL_INIT} CACHE STRING
     "Flags used by the linker during release minsize builds.")

  SET (CMAKE_EXE_LINKER_FLAGS_RELEASE ${CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT} "" CACHE STRING
     "Flags used by the linker during release builds.")

  SET (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO 
     ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT} CACHE STRING
     "Flags used by the linker during Release with Debug Info builds.")
  
  SET (CMAKE_SHARED_LINKER_FLAGS_DEBUG ${CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT} CACHE STRING
     "Flags used by the linker during debug builds.")

  SET (CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL ${CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL_INIT}
     CACHE STRING
     "Flags used by the linker during release minsize builds.")

  SET (CMAKE_SHARED_LINKER_FLAGS_RELEASE ${CMAKE_SHARED_LINKER_FLAGS_RELEASE_INIT} "" CACHE STRING
     "Flags used by the linker during release builds.")

  SET (CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO 
     ${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO_INIT} CACHE STRING
     "Flags used by the linker during Release with Debug Info builds.")

  SET (CMAKE_MODULE_LINKER_FLAGS_DEBUG ${CMAKE_MODULE_LINKER_FLAGS_DEBUG_INIT} CACHE STRING
     "Flags used by the linker during debug builds.")

  SET (CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL ${CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL_INIT}
     CACHE STRING
     "Flags used by the linker during release minsize builds.")

  SET (CMAKE_MODULE_LINKER_FLAGS_RELEASE ${CMAKE_MODULE_LINKER_FLAGS_RELEASE_INIT} "" CACHE STRING
     "Flags used by the linker during release builds.")

  SET (CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO 
     ${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO_INIT} CACHE STRING
     "Flags used by the linker during Release with Debug Info builds.")
ENDIF(NOT CMAKE_NOT_USING_CONFIG_FLAGS)



# shared linker flags
SET (CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS_INIT} $ENV{LDFLAGS}
     CACHE STRING "Flags used by the linker during the creation of dll's.")

# module linker flags
SET (CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS_INIT} $ENV{LDFLAGS}
     CACHE STRING "Flags used by the linker during the creation of modules.")

SET(CMAKE_BUILD_TOOL ${CMAKE_MAKE_PROGRAM} CACHE INTERNAL 
     "What is the target build tool cmake is generating for.")

MARK_AS_ADVANCED(
CMAKE_BUILD_TOOL
CMAKE_VERBOSE_MAKEFILE 
CMAKE_CXX_FLAGS
CMAKE_CXX_FLAGS_RELEASE
CMAKE_CXX_FLAGS_RELWITHDEBINFO
CMAKE_CXX_FLAGS_MINSIZEREL
CMAKE_CXX_FLAGS_DEBUG

CMAKE_C_FLAGS
CMAKE_C_FLAGS_DEBUG
CMAKE_C_FLAGS_MINSIZEREL
CMAKE_C_FLAGS_RELEASE
CMAKE_C_FLAGS_RELWITHDEBINFO

CMAKE_EXE_LINKER_FLAGS
CMAKE_EXE_LINKER_FLAGS_DEBUG
CMAKE_EXE_LINKER_FLAGS_MINSIZEREL
CMAKE_EXE_LINKER_FLAGS_RELEASE
CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO

CMAKE_SHARED_LINKER_FLAGS
CMAKE_SHARED_LINKER_FLAGS_DEBUG
CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL
CMAKE_SHARED_LINKER_FLAGS_RELEASE
CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO

CMAKE_MODULE_LINKER_FLAGS
CMAKE_MODULE_LINKER_FLAGS_DEBUG
CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL
CMAKE_MODULE_LINKER_FLAGS_RELEASE
CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO

)
IF(NOT UNIX)
  MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)
ENDIF(NOT UNIX)

SET(CMAKE_SYSTEM_SPECIFIC_INFORMATION_LOADED 1)
