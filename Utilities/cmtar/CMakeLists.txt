CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(LIBTAR C)

INCLUDE_REGULAR_EXPRESSION("^.*$")

# We need ansi c-flags, especially on HP
SET(CMAKE_C_FLAGS "${CMAKE_ANSI_CFLAGS} ${CMAKE_C_FLAGS}")
SET(CMAKE_REQUIRED_FLAGS ${CMAKE_ANSI_CFLAGS})

# Disable warnings on Borland to avoid changing 3rd party code.
IF(BORLAND)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w-")
ENDIF(BORLAND)

# If we are on AIX, do the _ALL_SOURCE magic
IF(${CMAKE_SYSTEM_NAME} MATCHES AIX)
  SET(_ALL_SOURCE 1)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES AIX)

# Include all the necessary files for macros
#SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
# Include all the necessary files for macros
INCLUDE (CheckIncludeFiles)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckTypeSize)
INCLUDE (CheckSymbolExists)
INCLUDE (TestBigEndian)

MACRO(MANGLE_VARIABLE_NAME str var prefix)
  STRING(TOUPPER "${str}" mangle_variable_name_var)
  STRING(REGEX REPLACE "[/. ]" "_" mangle_variable_name_var "${mangle_variable_name_var}")
  SET(${var} "${prefix}${mangle_variable_name_var}")
ENDMACRO(MANGLE_VARIABLE_NAME str var)

# Check if header file exists and add it to the list.
MACRO(CHECK_INCLUDE_FILE_CONCAT FILE)
  MANGLE_VARIABLE_NAME("${FILE}" "CHECK_INCLUDE_FILE_CONCAT_VAR" "HAVE_")
  CHECK_INCLUDE_FILES("${HEADER_INCLUDES};${FILE}" ${CHECK_INCLUDE_FILE_CONCAT_VAR})
  IF(${CHECK_INCLUDE_FILE_CONCAT_VAR})
    SET(HEADER_INCLUDES ${HEADER_INCLUDES} ${FILE})
  ENDIF(${CHECK_INCLUDE_FILE_CONCAT_VAR})
ENDMACRO(CHECK_INCLUDE_FILE_CONCAT)

MACRO(CHECK_FUNCTION_EXISTS_EX FUNC)
  MANGLE_VARIABLE_NAME("${FUNC}" "CHECK_FUNCTION_EXISTS_EX_VAR" "HAVE_")
  CHECK_FUNCTION_EXISTS("${FUNC}" "${CHECK_FUNCTION_EXISTS_EX_VAR}")
ENDMACRO(CHECK_FUNCTION_EXISTS_EX)

MACRO(CHECK_SYMBOL_EXISTS_EX SYM)
  MANGLE_VARIABLE_NAME("${SYM}" "CHECK_SYMBOL_EXISTS_EX_VAR" "HAVE_")
  CHECK_SYMBOL_EXISTS("${SYM}" "${HEADER_INCLUDES}" "${CHECK_SYMBOL_EXISTS_EX_VAR}")
ENDMACRO(CHECK_SYMBOL_EXISTS_EX)

#MACRO(CHECK_TYPE_SIZE_EX type defualt_size)
#  MANGLE_VARIABLE_NAME("${type}" "check_type_size_var" "")
#  CHECK_TYPE_SIZE("${type}" "SIZEOF_${check_type_size_var}")
#  IF(HAVE_${check_type_size_var})
#    SET("HAVE_${check_type_size_var}" 1)
#  ELSE(HAVE_${check_type_size_var})
#  ENDIF(HAVE_${check_type_size_var})
#ENDMACRO(CHECK_TYPE_SIZE_EX)



INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

#check for stdc headers
CHECK_INCLUDE_FILES("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)

#check for other headers used by the program
FOREACH(file
  "ctype.h"
  "fnmatch.h"
  "inttypes.h"
  "io.h"
  "libgen.h"
  "memory.h"
  "sys/mkdev.h"
  "stdint.h"
  "stdlib.h"
  "stdio.h"
  "string.h"
  "strings.h"
  "sys/param.h"
  "sys/types.h"
  "sys/stat.h"
  "unistd.h"
  "glob.h"
  "dirent.h"
  "sys/sysmacros.h"
  )
  CHECK_INCLUDE_FILE_CONCAT("${file}")
ENDFOREACH(file)

#check for the functions used by the program
FOREACH(func
    basename
    dirname
    fnmatch
    lchown
    snprintf
    strlcpy
    strmode
    strsep
    strdup
    strftime
    vsnprintf
    glob
    major
    minor
    makedev
    )
  CHECK_SYMBOL_EXISTS_EX("${func}")
ENDFOREACH(func)

# on Syllable lchown() is there, but always returns "Not implemented"
IF("${CMAKE_SYSTEM_NAME}" MATCHES syllable)
  SET(HAVE_LCHOWN 0)
ENDIF("${CMAKE_SYSTEM_NAME}" MATCHES syllable)

CHECK_TYPE_SIZE("dev_t" SIZEOF_DEV_T)
IF(HAVE_SIZEOF_DEV_T)
  SET (HAVE_DEV_T 1)
ELSE(HAVE_SIZEOF_DEV_T)
  SET (HAVE_DEV_T 0)
  SET (dev_t "unsigned long")
ENDIF(HAVE_SIZEOF_DEV_T)

CHECK_TYPE_SIZE("major_t" SIZEOF_MAJOR_T)
IF(HAVE_SIZEOF_MAJOR_T)
  SET (HAVE_MAJOR_T 1)
ELSE(HAVE_SIZEOF_MAJOR_T)
  SET (HAVE_MAJOR_T 0)
  SET (major_t "unsigned int")
ENDIF(HAVE_SIZEOF_MAJOR_T)

CHECK_TYPE_SIZE("minor_t" SIZEOF_MINOR_T)
IF(HAVE_SIZEOF_MINOR_T)
  SET (HAVE_MINOR_T 1)
ELSE(HAVE_SIZEOF_MINOR_T)
  SET (HAVE_MINOR_T 0)
  SET (minor_t "unsigned int")
ENDIF(HAVE_SIZEOF_MINOR_T)

CHECK_TYPE_SIZE("nlink_t" SIZEOF_NLINK_T)
IF(HAVE_SIZEOF_NLINK_T)
  SET (HAVE_NLINK_T 1)
ELSE(HAVE_SIZEOF_NLINK_T)
  SET (HAVE_NLINK_T 0)
  SET (nlink_t "unsigned short")
ENDIF(HAVE_SIZEOF_NLINK_T)

CHECK_TYPE_SIZE("uint64_t" SIZEOF_UINT64_T)
IF(HAVE_SIZEOF_UINT64_T)
  SET (HAVE_UINT64_T 1)
ELSE(HAVE_SIZEOF_UINT64_T)
  SET (HAVE_UINT64_T 0)
  SET (uint64_t "long long")
ENDIF(HAVE_SIZEOF_UINT64_T)

CHECK_TYPE_SIZE("gid_t" SIZEOF_GID_T)
IF(HAVE_SIZEOF_GID_T)
  SET (HAVE_GID_T 1)
ELSE(HAVE_SIZEOF_GID_T)
  SET (HAVE_GID_T 0)
  SET (gid_t "int")
ENDIF(HAVE_SIZEOF_GID_T)

CHECK_TYPE_SIZE("mode_t" SIZEOF_MODE_T)
IF(HAVE_SIZEOF_MODE_T)
  SET (HAVE_MODE_T 1)
ELSE(HAVE_SIZEOF_MODE_T)
  SET (HAVE_MODE_T 0)
  SET (mode_t "int")
ENDIF(HAVE_SIZEOF_MODE_T)

CHECK_TYPE_SIZE("off_t" SIZEOF_OFF_T)
IF(HAVE_SIZEOF_OFF_T)
  SET (HAVE_OFF_T 1)
ELSE(HAVE_SIZEOF_OFF_T)
  SET (HAVE_OFF_T 0)
  SET (off_t "long")
ENDIF(HAVE_SIZEOF_OFF_T)
CHECK_TYPE_SIZE("size_t" SIZEOF_SIZE_T)
IF(HAVE_SIZEOF_SIZE_T)
  SET (HAVE_SIZE_T 1)
ELSE(HAVE_SIZEOF_SIZE_T)
  SET (HAVE_SIZE_T 0)
  SET (size_t "unsigned")
ENDIF(HAVE_SIZEOF_SIZE_T)

CHECK_TYPE_SIZE("ssize_t" SIZEOF_SSIZE_T)
IF(HAVE_SIZEOF_SSIZE_T)
  SET (HAVE_SSIZE_T 1)
ELSE(HAVE_SIZEOF_SSIZE_T)
  SET (HAVE_SSIZE_T 0)
  CHECK_TYPE_SIZE("long"  SIZEOF_LONG)
  CHECK_TYPE_SIZE("__int64"  SIZEOF___INT64)
  IF(SIZEOF_LONG EQUAL SIZEOF_SIZE_T)
    SET(ssize_t "long")
  ENDIF(SIZEOF_LONG EQUAL SIZEOF_SIZE_T)
  IF(NOT ssize_t AND SIZEOF___INT64 EQUAL SIZEOF_SIZE_T)
    SET(ssize_t "__int64")
  ENDIF(NOT ssize_t AND SIZEOF___INT64 EQUAL SIZEOF_SIZE_T)
ENDIF(HAVE_SIZEOF_SSIZE_T)

CHECK_TYPE_SIZE("uid_t" SIZEOF_UID_T)
IF(HAVE_SIZEOF_UID_T)
  SET (HAVE_UID_T 1)
ELSE(HAVE_SIZEOF_UID_T)
  SET (HAVE_UID_T 0)
  SET (uid_t "int")
ENDIF(HAVE_SIZEOF_UID_T)

#SET (HAVE_BASENAME 0)
SET (HAVE_CTYPE_H 1) # all our platforms have <ctype.h>
#SET (HAVE_DEV_T 1)
#SET (HAVE_DIRNAME 0)
#SET (HAVE_FNMATCH 1)
#SET (HAVE_FNMATCH_H 1)
#SET (HAVE_INTTYPES_H 1)
#SET (HAVE_LCHOWN 1)
#SET (HAVE_LIBGEN_H 1)
SET (HAVE_LIBZ 1)
#SET (HAVE_MAJOR_T 0)
#SET (HAVE_MEMORY_H 1)
#SET (HAVE_MINOR_T 0)
#SET (HAVE_NLINK_T 1)
#SET (HAVE_SNPRINTF 1)
#SET (HAVE_STDINT_H 1)
#SET (HAVE_STDLIB_H 1)
#SET (HAVE_STRDUP 1)
#SET (HAVE_STRFTIME 1)
#SET (HAVE_STRINGS_H 1)
#SET (HAVE_STRING_H 1)
#SET (HAVE_STRLCPY 0)
#SET (HAVE_STRMODE 0)
#SET (HAVE_STRSEP 1)
#SET (HAVE_SYS_STAT_H 1)
#SET (HAVE_SYS_TYPES_H 1)
#SET (HAVE_UINT64_T 1)
#SET (HAVE_UNISTD_H 1)
SET (MAJOR_IN_MKDEV 0)
SET (MAJOR_IN_SYSMACROS 0)

# for most systems makedev is two args on qnx it is three
# try run is nothing but trouble so just hard code it 
SET (MAKEDEV_THREE_ARGS 0)
IF(QNXNTO)
  SET(MAKEDEV_THREE_ARGS 1)
ENDIF(QNXNTO)
#SET (NEED_BASENAME 0)
#SET (NEED_DIRNAME 0)
#SET (NEED_FNMATCH 1)
IF(UNIX)
  SET (NEED_MAKEDEV 1)
ENDIF(UNIX)
#SET (NEED_SNPRINTF 0)
#SET (NEED_STRDUP 0)
#SET (NEED_STRLCPY 0)
#SET (NEED_STRMODE 1)
#SET (NEED_STRSEP 0)
SET (PACKAGE_BUGREPORT "")
SET (PACKAGE_NAME "\"libtar\"")
SET (PACKAGE_STRING "\"libtar 1.2.11\"")
SET (PACKAGE_TARNAME "\"libtar\"")
SET (PACKAGE_VERSION "\"1.2.11\"")
#SET (STDC_HEADERS 1)
SET (const 0)
#SET (dev_t 0)
#SET (gid_t 0)
#SET (major_t "unsigned int")
#SET (minor_t "unsigned int")
#SET (mode_t 0)
#SET (nlink_t 0)
#SET (off_t 0)
#SET (size_t 0)
#SET (uid_t 0)
#SET (uint64_t 0)
SET (LISTHASH_PREFIX "libtar")

CONFIGURE_FILE(${LIBTAR_SOURCE_DIR}/listhash/listhash.h.in
  ${LIBTAR_BINARY_DIR}/libtar/libtar_listhash.h @ONLY IMMEDIATE)
CONFIGURE_FILE(${LIBTAR_SOURCE_DIR}/listhash/list.c.in
  ${LIBTAR_BINARY_DIR}/listhash/libtar_list.c @ONLY IMMEDIATE)
CONFIGURE_FILE(${LIBTAR_SOURCE_DIR}/listhash/hash.c.in
  ${LIBTAR_BINARY_DIR}/listhash/libtar_hash.c @ONLY IMMEDIATE)

FOREACH(hfile libtar.h tar.h compat/compat.h)
  GET_FILENAME_COMPONENT(outname "${hfile}" NAME)
  CONFIGURE_FILE(${LIBTAR_SOURCE_DIR}/${hfile}
    ${LIBTAR_BINARY_DIR}/libtar/${outname} @ONLY IMMEDIATE)
ENDFOREACH(hfile)

FOREACH(hfile internal.h filesystem.h)
  GET_FILENAME_COMPONENT(outname "${hfile}" NAME)
  CONFIGURE_FILE(${LIBTAR_SOURCE_DIR}/${hfile}
    ${LIBTAR_BINARY_DIR}/libtarint/${outname} @ONLY IMMEDIATE)
ENDFOREACH(hfile)
  
SET(libtar_SRC
  append.c
  block.c
  decode.c
  encode.c
  extract.c
  handle.c
  ${LIBTAR_BINARY_DIR}/listhash/libtar_hash.c
  ${LIBTAR_BINARY_DIR}/listhash/libtar_list.c
  output.c
  util.c
  wrapper.c
  internal.h
  ${LIBTAR_BINARY_DIR}/libtar/config.h
  config.h.in
  tar.h
  libtar.h
  compat/compat.h
  
#  compat/strlcpy.c
#  compat/basename.c
#  compat/dirname.c
#  compat/strmode.c 
#  compat/strsep.c 
)
IF(NOT HAVE_DIRENT_H)
  SET(libtar_SRC
    ${libtar_SRC}
    filesystem.c filesystem.h)
ENDIF(NOT HAVE_DIRENT_H)

IF(NOT HAVE_STRLCPY)
SET(libtar_SRC ${libtar_SRC} compat/strlcpy.c)
SET(NEED_STRLCPY 1)
ENDIF(NOT HAVE_STRLCPY)

IF(NOT HAVE_STRMODE)
SET(libtar_SRC ${libtar_SRC} compat/strmode.c)
SET(NEED_STRMODE 1)
ENDIF(NOT HAVE_STRMODE)

IF(WIN32)
IF(NOT HAVE_SNPRINTF OR NOT HAVE_VSNPRINTF)
SET(libtar_SRC ${libtar_SRC} compat/snprintf.c)
SET(NEED_SNPRINTF 1)
ENDIF(NOT HAVE_SNPRINTF OR NOT HAVE_VSNPRINTF)
ENDIF(WIN32)

IF(NOT HAVE_DIRNAME)
SET(libtar_SRC ${libtar_SRC} compat/dirname.c)
SET(NEED_DIRNAME 1)
ENDIF(NOT HAVE_DIRNAME)


IF(NOT HAVE_STRSEP)
SET(libtar_SRC ${libtar_SRC} compat/strsep.c)
SET(NEED_STRSEP 1)
ENDIF(NOT HAVE_STRSEP)


IF(NOT HAVE_BASENAME)
SET(libtar_SRC ${libtar_SRC} compat/basename.c)
SET(NEED_BASENAME 1)
ENDIF(NOT HAVE_BASENAME)

IF(NOT HAVE_FNMATCH)
SET(libtar_SRC ${libtar_SRC} compat/fnmatch.c)
SET(NEED_FNMATCH 1)
ENDIF(NOT HAVE_FNMATCH)

#IF(NOT HAVE_GLOB)
#SET(libtar_SRC ${libtar_SRC} compat/glob.c)
#SET(NEED_GLOB 1)
#ENDIF(NOT HAVE_GLOB)

# Setup zlib.
IF(NOT CMTAR_ZLIB_LIBRARIES)
  MESSAGE(FATAL_ERROR "Parent project must set CMTAR_ZLIB_LIBRARIES.")
ENDIF(NOT CMTAR_ZLIB_LIBRARIES)
IF(NOT CMTAR_ZLIB_HEADER)
  MESSAGE(FATAL_ERROR "Parent project must set CMTAR_ZLIB_HEADER.")
ENDIF(NOT CMTAR_ZLIB_HEADER)
INCLUDE_DIRECTORIES(${CMTAR_ZLIB_INCLUDE_DIRS})

CONFIGURE_FILE(${LIBTAR_SOURCE_DIR}/config.h.in
  ${LIBTAR_BINARY_DIR}/libtar/config.h)

ADD_LIBRARY(cmtar STATIC ${libtar_SRC})
ADD_EXECUTABLE(tartest libtar.c)
TARGET_LINK_LIBRARIES(tartest cmtar ${CMTAR_ZLIB_LIBRARIES})
