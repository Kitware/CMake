
# This module is used by the Makefile generator to determin the following variables:
# CMAKE_SYSTEM_NAME - on unix this is uname -s, for windows it is Windows
# CMAKE_SYSTEM_VERSION - on unix this is uname -r, for windows it is empty
# CMAKE_SYSTEM - ${CMAKE_SYSTEM}-${CMAKE_SYSTEM_VERSION}, for windows: ${CMAKE_SYSTEM}
#
#  Expected uname -s output:
#
# AIX                           AIX  
# BSD/OS                        BSD/OS  
# FreeBSD                       FreeBSD  
# HP-UX                         HP-UX  
# IRIX                          IRIX  
# Linux                         Linux  
# NetBSD                        NetBSD  
# OpenBSD                       OpenBSD  
# OFS/1 (Digital Unix)          OSF1  
# SCO OpenServer 5              SCO_SV  
# SCO UnixWare 7                UnixWare  
# SCO UnixWare (pre release 7)  UNIX_SV  
# SCO XENIX                     Xenix  
# Solaris                       SunOS  
# SunOS                         SunOS  
# Tru64                         Tru64  
# Ultrix                        ULTRIX  
# cygwin                        CYGWIN_NT-5.1
# MacOSX                        Darwin
  
IF(UNIX)
  FIND_PROGRAM(CMAKE_UNAME uname /bin /usr/bin /usr/local/bin )
  IF(CMAKE_UNAME)
    EXEC_PROGRAM(uname ARGS -s OUTPUT_VARIABLE CMAKE_SYSTEM_NAME)
    EXEC_PROGRAM(uname ARGS -r OUTPUT_VARIABLE CMAKE_SYSTEM_VERSION)
    EXEC_PROGRAM(uname ARGS -p OUTPUT_VARIABLE CMAKE_SYSTEM_PROCESSOR
      RETURN_VALUE val)
    IF("${val}" GREATER 0)
      EXEC_PROGRAM(uname ARGS -m OUTPUT_VARIABLE CMAKE_SYSTEM_PROCESSOR
        RETURN_VALUE val)
    ENDIF("${val}" GREATER 0)
    IF("${val}" GREATER 0)
      SET(CMAKE_SYSTEM_PROCESSOR "unknown")
    ENDIF("${val}" GREATER 0)
    SET(CMAKE_UNAME ${CMAKE_UNAME} CACHE INTERNAL "uname command")
    # processor may have double quote in the name, and that needs to be removed
    STRING(REGEX REPLACE "\"" "" CMAKE_SYSTEM_PROCESSOR "${CMAKE_SYSTEM_PROCESSOR}")
    STRING(REGEX REPLACE "/" "_" CMAKE_SYSTEM_PROCESSOR "${CMAKE_SYSTEM_PROCESSOR}")
  ENDIF(CMAKE_UNAME)
ELSE(UNIX)
  IF(WIN32)
    SET (CMAKE_SYSTEM_NAME "Windows")
    SET (CMAKE_SYSTEM_PROCESSOR "$ENV{PROCESSOR_ARCHITECTURE}")
  ENDIF(WIN32)
ENDIF(UNIX)

IF(NOT CMAKE_SYSTEM_NAME)
  SET(CMAKE_SYSTEM_NAME "UnknownOS")
ENDIF(NOT CMAKE_SYSTEM_NAME)

# fix for BSD/OS , remove the /
IF(CMAKE_SYSTEM_NAME MATCHES BSD.OS)
  SET(CMAKE_SYSTEM_NAME BSDOS)
ENDIF(CMAKE_SYSTEM_NAME MATCHES BSD.OS)

# fix for CYGWIN which has windows version in it 
IF(CMAKE_SYSTEM_NAME MATCHES CYGWIN)
  SET(CMAKE_SYSTEM_NAME CYGWIN)
ENDIF(CMAKE_SYSTEM_NAME MATCHES CYGWIN)

# set CMAKE_SYSTEM to the CMAKE_SYSTEM_NAME
SET(CMAKE_SYSTEM  ${CMAKE_SYSTEM_NAME})
# if there is a CMAKE_SYSTEM_VERSION then add a -${CMAKE_SYSTEM_VERSION}
IF(CMAKE_SYSTEM_VERSION)
  SET(CMAKE_SYSTEM ${CMAKE_SYSTEM}-${CMAKE_SYSTEM_VERSION})
ENDIF(CMAKE_SYSTEM_VERSION)


FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log 
  "The system is: ${CMAKE_SYSTEM_NAME} - ${CMAKE_SYSTEM_VERSION} - ${CMAKE_SYSTEM_PROCESSOR}\n")

# configure variables set in this file for fast reload
CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CMakeSystem.cmake.in 
               ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeSystem.cmake 
               IMMEDIATE)

