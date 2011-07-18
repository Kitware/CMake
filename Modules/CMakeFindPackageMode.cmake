# COMPILER_ID = GNU/Intel/etc.
# LANGUAGE = C/CXX/Fortan/ASM
# MODE = EXIST/COMPILE/LINK
# NAME = name of the package
# QUIET = if TRUE, don't print anything

#=============================================================================
# Copyright 2006-2011 Alexander Neundorf, <neundorf@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

if(NOT NAME)
  message(FATAL_ERROR "NAME argument not specified.")
endif()

if(NOT COMPILER_ID)
  message(FATAL_ERROR "COMPILER_ID argument not specified. In doubt, use GNU.")
endif()

if(NOT LANGUAGE)
  message(FATAL_ERROR "LANGUAGE argument not specified. Use C, CXX or Fortran.")
endif()

if(NOT MODE)
  message(FATAL_ERROR "MODE argument not specified. Use either EXIST, COMPILE or LINK.")
endif()


include(CMakeDetermineSystem)

# Also load the system specific file, which sets up e.g. the search paths.
# This makes the FIND_XXX() calls work much better
include(CMakeSystemSpecificInformation)

if(UNIX)

  # try to guess whether we have a 64bit system, if it has not been set
  # from the outside
  if(NOT CMAKE_SIZEOF_VOID_P)
    set(CMAKE_SIZEOF_VOID_P 4)
    if(EXISTS /usr/lib64)
      set(CMAKE_SIZEOF_VOID_P 8)
    else()
      # use the file utility to check whether itself is 64 bit:
      find_program(FILE_EXECUTABLE file)
      if(FILE_EXECUTABLE)
        execute_process(COMMAND "${FILE_EXECUTABLE}" "${FILE_EXECUTABLE}" OUTPUT_VARIABLE fileOutput ERROR_QUIET)
        if("${fileOutput}" MATCHES "64-bit")
          set(CMAKE_SIZEOF_VOID_P 8)
        endif()
      endif()
    endif()
  endif()

  # guess Debian multiarch if it has not been set:
  if(EXISTS /etc/debian_version)
    if(NOT CMAKE_${LANGUAGE}_LANGUAGE_ARCHITECTURE )
      file(GLOB filesInLib RELATIVE /lib /lib/*-linux-gnu* )
      foreach(file ${filesInLib})
        if("${file}" MATCHES "${CMAKE_LIBRARY_ARCHITECTURE_REGEX}")
          set(CMAKE_${LANGUAGE}_LANGUAGE_ARCHITECTURE ${file})
          break()
        endif()
      endforeach()
    endif()
  endif()

endif()

set(CMAKE_${LANGUAGE}_COMPILER "dummy")
set(CMAKE_${LANGUAGE}_COMPILER_ID "${COMPILER_ID}")
include(CMake${LANGUAGE}Information)


function(print_compile_flags _packageName)
  string(TOUPPER "${_packageName}" PACKAGE_NAME)
  # Check the following variables:
  # FOO_INCLUDE_DIRS
  # Foo_INCLUDE_DIRS
  # FOO_INCLUDES
  # Foo_INCLUDES
  # FOO_INCLUDE_DIR
  # Foo_INCLUDE_DIR
  set(includes)
  if(DEFINED ${_packageName}_INCLUDE_DIRS)
    set(includes ${_packageName}_INCLUDE_DIRS)
  elseif(DEFINED ${PACKAGE_NAME}_INCLUDE_DIRS)
    set(includes ${PACKAGE_NAME}_INCLUDE_DIRS)
  elseif(DEFINED ${_packageName}_INCLUDES)
    set(includes ${_packageName}_INCLUDES)
  elseif(DEFINED ${PACKAGE_NAME}_INCLUDES)
    set(includes ${PACKAGE_NAME}_INCLUDES)
  elseif(DEFINED ${_packageName}_INCLUDE_DIR)
    set(includes ${_packageName}_INCLUDE_DIR)
  elseif(DEFINED ${PACKAGE_NAME}_INCLUDE_DIR)
    set(includes ${PACKAGE_NAME}_INCLUDE_DIR)
  endif()

  set(PACKAGE_INCLUDE_DIRS "${${includes}}" PARENT_SCOPE)

  # Check the following variables:
  # FOO_DEFINITIONS
  # Foo_DEFINITIONS
  set(definitions)
  if(DEFINED ${_packageName}_DEFINITIONS)
    set(definitions ${_packageName}_DEFINITIONS)
  elseif(DEFINED ${PACKAGE_NAME}_DEFINITIONS)
    set(definitions ${PACKAGE_NAME}_DEFINITIONS)
  endif()

  set(PACKAGE_DEFINITIONS  "${${definitions}}" )

endfunction()


function(print_link_flags _packageName)
  string(TOUPPER "${_packageName}" PACKAGE_NAME)
  # Check the following variables:
  # FOO_LIBRARIES
  # Foo_LIBRARIES
  # FOO_LIBS
  # Foo_LIBS
  set(libs)
  if(DEFINED ${_packageName}_LIBRARIES)
    set(libs ${_packageName}_LIBRARIES)
  elseif(DEFINED ${PACKAGE_NAME}_LIBRARIES)
    set(libs ${PACKAGE_NAME}_LIBRARIES)
  elseif(DEFINED ${_packageName}_LIBS)
    set(libs ${_packageName}_LIBS)
  elseif(DEFINED ${PACKAGE_NAME}_LIBS)
    set(libs ${PACKAGE_NAME}_LIBS)
  endif()

  set(PACKAGE_LIBRARIES "${${libs}}" PARENT_SCOPE )

endfunction()


find_package("${NAME}" QUIET)

set(PACKAGE_FOUND FALSE)

string(TOUPPER "${NAME}" UPPERCASE_NAME)

if(${NAME}_FOUND  OR  ${UPPERCASE_NAME}_FOUND)
  set(PACKAGE_FOUND TRUE)

  if("${MODE}" STREQUAL "EXIST")
    # do nothing
  elseif("${MODE}" STREQUAL "COMPILE")
    print_compile_flags(${NAME})
  elseif("${MODE}" STREQUAL "LINK")
    print_link_flags(${NAME})
  else("${MODE}" STREQUAL "LINK")
    message(FATAL_ERROR "Invalid mode argument ${MODE} given.")
  endif()

endif()

set(PACKAGE_QUIET ${SILENT} )
