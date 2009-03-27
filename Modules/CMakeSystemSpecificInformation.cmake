
# This file is included by cmGlobalGenerator::EnableLanguage.
# It is included after the compiler has been determined, so
# we know things like the compiler name and if the compiler is gnu.

# before cmake 2.6 these variables were set in cmMakefile.cxx. This is still
# done to keep scripts and custom language and compiler modules working.
# But they are reset here and set again in the platform files for the target
# platform, so they can be used for testing the target platform instead
# of testing the host platform.
SET(APPLE  )
SET(UNIX   )
SET(CYGWIN )
SET(WIN32  )


# include Generic system information
INCLUDE(CMakeGenericSystem)

# 2. now include SystemName.cmake file to set the system specific information
SET(CMAKE_SYSTEM_INFO_FILE Platform/${CMAKE_SYSTEM_NAME})

INCLUDE(${CMAKE_SYSTEM_INFO_FILE} OPTIONAL RESULT_VARIABLE _INCLUDED_SYSTEM_INFO_FILE)

IF(NOT _INCLUDED_SYSTEM_INFO_FILE)
  MESSAGE("System is unknown to cmake, create:\n${CMAKE_SYSTEM_INFO_FILE}"
          " to use this system, please send your config file to "
          "cmake@www.cmake.org so it can be added to cmake")
  IF(EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
    CONFIGURE_FILE(${CMAKE_BINARY_DIR}/CMakeCache.txt
                   ${CMAKE_BINARY_DIR}/CopyOfCMakeCache.txt COPYONLY)
    MESSAGE("Your CMakeCache.txt file was copied to CopyOfCMakeCache.txt. " 
            "Please send that file to cmake@www.cmake.org.")
   ENDIF(EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
ENDIF(NOT _INCLUDED_SYSTEM_INFO_FILE)


# The Eclipse generator needs to know the standard include path
# so that Eclipse ca find the headers at runtime and parsing etc. works better
# This is done here by actually running gcc with the options so it prints its
# system include directories, which are parsed then and stored in the cache.
IF("${CMAKE_EXTRA_GENERATOR}" MATCHES "Eclipse")

  MACRO(_DETERMINE_GCC_SYSTEM_INCLUDE_DIRS _lang _result)
    SET(${_result})
    SET(_gccOutput)
    FILE(WRITE "${CMAKE_BINARY_DIR}/CMakeFiles/dummy" "\n" )
    EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} -v -E -x ${_lang} dummy
                   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/CMakeFiles
                   ERROR_VARIABLE _gccOutput
                   OUTPUT_QUIET )
    FILE(REMOVE "${CMAKE_BINARY_DIR}/CMakeFiles/dummy")

    IF( "${_gccOutput}" MATCHES "> search starts here[^\n]+\n *(.+) *\n *End of (search) list" )
      SET(${_result} ${CMAKE_MATCH_1})
      STRING(REPLACE "\n" " " ${_result} "${${_result}}")
      SEPARATE_ARGUMENTS(${_result})
    ENDIF( "${_gccOutput}" MATCHES "> search starts here[^\n]+\n *(.+) *\n *End of (search) list" )
  ENDMACRO(_DETERMINE_GCC_SYSTEM_INCLUDE_DIRS _lang)

  # Now check for C
  IF ("${CMAKE_C_COMPILER_ID}" MATCHES GNU  AND NOT  CMAKE_ECLIPSE_C_SYSTEM_INCLUDE_DIRS)
    _DETERMINE_GCC_SYSTEM_INCLUDE_DIRS(c _dirs)
    SET(CMAKE_ECLIPSE_C_SYSTEM_INCLUDE_DIRS "${_dirs}" CACHE INTERNAL "C compiler system include directories")
  ENDIF ("${CMAKE_C_COMPILER_ID}" MATCHES GNU  AND NOT  CMAKE_ECLIPSE_C_SYSTEM_INCLUDE_DIRS)

  # And now the same for C++
  IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES GNU  AND NOT  CMAKE_ECLIPSE_CXX_SYSTEM_INCLUDE_DIRS)
    _DETERMINE_GCC_SYSTEM_INCLUDE_DIRS(c++ _dirs)
    SET(CMAKE_ECLIPSE_CXX_SYSTEM_INCLUDE_DIRS "${_dirs}" CACHE INTERNAL "CXX compiler system include directories")
  ENDIF ("${CMAKE_CXX_COMPILER_ID}" MATCHES GNU  AND NOT  CMAKE_ECLIPSE_CXX_SYSTEM_INCLUDE_DIRS)

ENDIF("${CMAKE_EXTRA_GENERATOR}" MATCHES "Eclipse")


# for most systems a module is the same as a shared library
# so unless the variable CMAKE_MODULE_EXISTS is set just
# copy the values from the LIBRARY variables
# this has to be done after the system information has been loaded
IF(NOT CMAKE_MODULE_EXISTS)
  SET(CMAKE_SHARED_MODULE_PREFIX "${CMAKE_SHARED_LIBRARY_PREFIX}")
  SET(CMAKE_SHARED_MODULE_SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
  SET(CMAKE_SHARED_MODULE_RUNTIME_C_FLAG ${CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG})
  SET(CMAKE_SHARED_MODULE_RUNTIME_C_FLAG_SEP ${CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP})
ENDIF(NOT CMAKE_MODULE_EXISTS)


SET(CMAKE_SYSTEM_SPECIFIC_INFORMATION_LOADED 1)
